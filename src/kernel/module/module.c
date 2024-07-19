#include <kernel/aslr/aslr.h>
#include <kernel/elf/structures.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/lock/mutex.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/module/module.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/signature/signature.h>
#include <kernel/symbol/symbol.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "module"



#define MODULE_ROOT_DIRECTORY "/boot/module"



typedef struct _MODULE_REGION{
	u64 base;
	u64 size;
} module_region_t;



typedef struct _MODULE_LOADER_CONTEXT{
	const char* name;
	module_t* module;
	const module_descriptor_t* module_descriptor;
	void* data;
	const elf_hdr_t* elf_header;
	const char* elf_string_table;
	elf_sym_t* elf_symbol_table;
	u64 elf_symbol_table_size;
	const char* elf_symbol_string_table;
	module_region_t elf_region_ue;
	module_region_t elf_region_ur;
	module_region_t elf_region_uw;
	module_region_t elf_region_iw;
} module_loader_context_t;



static omm_allocator_t* KERNEL_INIT_WRITE _module_allocator=NULL;
static mmap_t* KERNEL_INIT_WRITE _module_image_mmap=NULL;
static mutex_t* KERNEL_INIT_WRITE _module_load_lock=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE module_handle_type=0;
KERNEL_PUBLIC notification_dispatcher_t* KERNEL_INIT_WRITE module_notification_dispatcher=NULL;



static void _module_handle_destructor(handle_t* handle){
	module_t* module=KERNEL_CONTAINEROF(handle,module_t,handle);
	smm_dealloc(module->name);
	event_delete(module->load_event);
	omm_dealloc(_module_allocator,module);
}



static void _find_static_elf_sections(module_loader_context_t* ctx){
	INFO("Locating static ELF sections...");
	ctx->elf_header=ctx->data;
	const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+ctx->elf_header->e_shstrndx*ctx->elf_header->e_shentsize;
	ctx->elf_string_table=ctx->data+section_header->sh_offset;
}



static bool _check_elf_header(module_loader_context_t* ctx){
	if (ctx->elf_header->e_ident.signature!=0x464c457f||ctx->elf_header->e_ident.word_size!=2||ctx->elf_header->e_ident.endianess!=1||ctx->elf_header->e_ident.header_version!=1||ctx->elf_header->e_ident.abi!=0||ctx->elf_header->e_type!=ET_REL||ctx->elf_header->e_machine!=0x3e||ctx->elf_header->e_version!=1){
		ERROR("Invalid ELF header");
		return 0;
	}
	return 1;
}



static KERNEL_AWAITS bool _map_sections(module_loader_context_t* ctx){
	INFO("Mapping sections...");
	u64 region_size=0;
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!(section_header->sh_flags&SHF_ALLOC)){
			continue;
		}
		u64 section_end=pmm_align_up_address(section_header->sh_addr+section_header->sh_size);
		if (section_end>region_size){
			region_size=section_end;
		}
	}
	INFO("Region size: %v",region_size);
	ctx->module->region=mmap_alloc(_module_image_mmap,0,region_size,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_WRITE,NULL);
	if (!ctx->module->region){
		ERROR("Unable to reserve module memory");
		return 0;
	}
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!(section_header->sh_flags&SHF_ALLOC)){
			section_header->sh_addr=0;
			continue;
		}
		section_header->sh_addr+=ctx->module->region->rb_node.key;
		for (volatile u8* ptr=(void*)pmm_align_down_address(section_header->sh_addr);ptr<(volatile u8*)(section_header->sh_addr+section_header->sh_size);ptr+=PAGE_SIZE){
			(void)(*ptr);
		}
		if (section_header->sh_type==SHT_PROGBITS){
			mem_copy((void*)(section_header->sh_addr),ctx->data+section_header->sh_offset,section_header->sh_size);
		}
	}
	ctx->module->region->flags|=MMAP_REGION_FLAG_NO_ALLOC;
	return 1;
}



static bool _find_elf_sections(module_loader_context_t* ctx){
	INFO("Locating ELF sections...");
	ctx->elf_symbol_table=NULL;
	ctx->elf_region_ue.base=0;
	ctx->elf_region_ue.size=0;
	ctx->elf_region_ur.base=0;
	ctx->elf_region_ur.size=0;
	ctx->elf_region_uw.base=0;
	ctx->elf_region_uw.size=0;
	ctx->elf_region_iw.base=0;
	ctx->elf_region_iw.size=0;
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (section_header->sh_type==SHT_SYMTAB){
			ctx->elf_symbol_table=ctx->data+section_header->sh_offset;
			ctx->elf_symbol_table_size=section_header->sh_size;
			section_header=ctx->data+ctx->elf_header->e_shoff+section_header->sh_link*ctx->elf_header->e_shentsize;
			ctx->elf_symbol_string_table=ctx->data+section_header->sh_offset;
			break;
		}
		else if (str_equal(ctx->elf_string_table+section_header->sh_name,".module_ue")){
			ctx->elf_region_ue.base=section_header->sh_addr;
			ctx->elf_region_ue.size=section_header->sh_size;
		}
		else if (str_equal(ctx->elf_string_table+section_header->sh_name,".module_ur")){
			ctx->elf_region_ur.base=section_header->sh_addr;
			ctx->elf_region_ur.size=section_header->sh_size;
		}
		else if (str_equal(ctx->elf_string_table+section_header->sh_name,".module_uw")){
			ctx->elf_region_uw.base=section_header->sh_addr;
			ctx->elf_region_uw.size=section_header->sh_size;
		}
		else if (str_equal(ctx->elf_string_table+section_header->sh_name,".module_iw")){
			ctx->elf_region_iw.base=section_header->sh_addr;
			ctx->elf_region_iw.size=section_header->sh_size;
		}
	}
	if (!ctx->elf_symbol_table){
		ERROR("'.symtab' section not present");
		return 0;
	}
	return 1;
}



static KERNEL_AWAITS bool _resolve_dependencies(module_loader_context_t* ctx){
	INFO("Resolving dependencies...");
	for (u64 i=1;i<ctx->elf_symbol_table_size/sizeof(elf_sym_t);i++){
		elf_sym_t* elf_symbol=ctx->elf_symbol_table+i;
		const char* name=ctx->elf_symbol_string_table+elf_symbol->st_name;
		if (str_equal(name,"__module_header")){
			ctx->module_descriptor=(void*)(elf_symbol->st_value+((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+elf_symbol->st_shndx*ctx->elf_header->e_shentsize))->sh_addr);
		}
	}
	if (!ctx->module_descriptor){
		ERROR("module header not present");
		return 0;
	}
	bool ret=1;
	for (u32 i=0;ctx->module_descriptor->dependencies[i];i++){
		module_t* module=module_load(ctx->module_descriptor->dependencies[i],1);
		if (!module||(module->state!=MODULE_STATE_LOADING&&module->state!=MODULE_STATE_LOADED)){
			ERROR("%s: failed dependency: %s",ctx->name,ctx->module_descriptor->dependencies[i]);
			ret=0;
		}
		if (module){
			event_await(&(module->load_event),1,0);
			handle_release(&(module->handle));
		}
	}
	return ret;
}



static bool _resolve_symbol_table(module_loader_context_t* ctx){
	INFO("Resolving symbol table...");
	bool ret=1;
	for (u64 i=1;i<ctx->elf_symbol_table_size/sizeof(elf_sym_t);i++){
		elf_sym_t* elf_symbol=ctx->elf_symbol_table+i;
		const char* name=ctx->elf_symbol_string_table+elf_symbol->st_name;
		if (elf_symbol->st_shndx==SHN_UNDEF&&name[0]){
			const symbol_t* symbol=symbol_lookup_by_name(name);
			if (symbol&&symbol->is_public){
				elf_symbol->st_value=symbol->rb_node.key;
			}
			else{
				ERROR("Unresolved symbol: %s",name);
				ret=0;
			}
		}
		else if (ret&&elf_symbol->st_shndx!=SHN_UNDEF&&(elf_symbol->st_info&0x0f)!=STT_SECTION&&(elf_symbol->st_info&0x0f)!=STT_FILE){
			u64 address=elf_symbol->st_value+((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+elf_symbol->st_shndx*ctx->elf_header->e_shentsize))->sh_addr;
			if (!str_starts_with(name,"__module")){
				symbol_add(ctx->module->name->data,name,address,(elf_symbol->st_info>>4)==STB_GLOBAL&&elf_symbol->st_other==STV_DEFAULT);
			}
		}
	}
	return ret;
}



static bool _apply_relocations(module_loader_context_t* ctx,bool resolve_undefined){
	INFO("Applying relocations...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!section_header->sh_info||section_header->sh_info>=ctx->elf_header->e_shnum||section_header->sh_type!=SHT_RELA){
			continue;
		}
		u64 base=((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+section_header->sh_info*ctx->elf_header->e_shentsize))->sh_addr;
		if (!base){
			continue;
		}
		const elf_rela_t* entry=ctx->data+section_header->sh_offset;
		for (u64 j=0;j<section_header->sh_size;j+=sizeof(elf_rela_t)){
			const elf_sym_t* symbol=ctx->elf_symbol_table+(entry->r_info>>32);
			if (resolve_undefined^(symbol->st_shndx==SHN_UNDEF)){
				entry++;
				continue;
			}
			u64 relocation_address=base+entry->r_offset;
			u64 value=symbol->st_value+entry->r_addend+((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+symbol->st_shndx*ctx->elf_header->e_shentsize))->sh_addr;
			switch (entry->r_info&0xffffffff){
				case R_X86_64_64:
					*((u64*)relocation_address)=value;
					break;
				case R_X86_64_PC32:
				case R_X86_64_PLT32:
					value-=relocation_address;
				case R_X86_64_32:
				case R_X86_64_32S:
					*((u32*)relocation_address)=value;
					break;
				default:
					ERROR("Unknown relocation type: %u",entry->r_info&0xffffffff);
					return 0;
			}
			entry++;
		}
	}
	return 1;
}



static void _adjust_memory_flags(module_loader_context_t* ctx){
	INFO("Adjusting memory flags...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!(section_header->sh_flags&SHF_ALLOC)){
			continue;
		}
		u64 address=pmm_align_down_address(section_header->sh_addr);
		u64 flags=0;
		if (section_header->sh_flags&SHF_WRITE){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		if (!(section_header->sh_flags&SHF_EXECINSTR)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		vmm_adjust_flags(&vmm_kernel_pagemap,address,flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(section_header->sh_size+section_header->sh_addr-address)>>PAGE_SIZE_SHIFT,1);
	}
}



static void _process_module_header(module_loader_context_t* ctx){
	INFO("Processing module header...");
	ctx->module->flags=ctx->module_descriptor->flags;
	*(ctx->module_descriptor->module_self_ptr)=ctx->module;
	ctx->module->deinit_array_base=ctx->module_descriptor->deinit.start;
	ctx->module->deinit_array_size=ctx->module_descriptor->deinit.end-ctx->module_descriptor->deinit.start;
#ifdef KERNEL_COVERAGE
	ctx->module->gcov_info_base=ctx->module_descriptor->gcov_info_start;
	ctx->module->gcov_info_size=ctx->module_descriptor->gcov_info_end-ctx->module_descriptor->gcov_info_start;
	if (ctx->module->gcov_info_size){
		INFO("Found .gcov_info data at %p (%u file%s)",ctx->module->gcov_info_base,ctx->module->gcov_info_size/sizeof(void*),(ctx->module->gcov_info_size/sizeof(void*)==1?"":"s"));
	}
	else{
		ctx->module->gcov_info_base=0;
	}
#endif
}



static void _unmap_region(module_loader_context_t* ctx,module_region_t* region){
	for (u64 address=pmm_align_down_address(region->base);address<pmm_align_up_address(region->base+region->size);address+=PAGE_SIZE){
		mmap_unmap_address(_module_image_mmap,address);
	}
}



static void _adjust_region_flags(module_loader_context_t* ctx,module_region_t* region){
	u64 address=pmm_align_down_address(region->base);
	vmm_adjust_flags(&vmm_kernel_pagemap,address,VMM_PAGE_FLAG_NOEXECUTE,VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(region->base+region->size-address)>>PAGE_SIZE_SHIFT,1);
}



static void _send_load_notification(module_t* module){
	module_load_notification_data_t* data=amm_alloc(sizeof(module_load_notification_data_t)+module->name->length+1);
	data->module_handle=module->handle.rb_node.key;
	mem_copy(data->name,module->name->data,module->name->length+1);
	notification_dispatcher_dispatch(module_notification_dispatcher,MODULE_LOAD_NOTIFICATION,data,sizeof(module_load_notification_data_t)+module->name->length+1);
	amm_dealloc(data);
}



static void _send_unload_notification(module_t* module){
	module_unload_notification_data_t* data=amm_alloc(sizeof(module_unload_notification_data_t)+module->name->length+1);
	data->module_handle=module->handle.rb_node.key;
	mem_copy(data->name,module->name->data,module->name->length+1);
	notification_dispatcher_dispatch(module_notification_dispatcher,MODULE_UNLOAD_NOTIFICATION,data,sizeof(module_unload_notification_data_t)+module->name->length+1);
	amm_dealloc(data);
}



static KERNEL_AWAITS void _async_initialization_thread(module_loader_context_t* ctx){
	if (!_map_sections(ctx)){
		goto _load_error;
	}
	if (!_find_elf_sections(ctx)){
		goto _load_error;
	}
	if (!_apply_relocations(ctx,0)){
		goto _load_error;
	}
	if (!_resolve_dependencies(ctx)){
		goto _load_error;
	}
	if (!_resolve_symbol_table(ctx)){
		goto _load_error;
	}
	if (!_apply_relocations(ctx,1)){
		goto _load_error;
	}
	_adjust_memory_flags(ctx);
	mmap_dealloc(process_kernel->mmap,(u64)(ctx->data),0);
	_process_module_header(ctx);
	_send_load_notification(ctx->module);
	INFO("Executing initializers...");
	for (u32 i=0;i<3;i++){
		for (u64 j=0;j+sizeof(void*)<=ctx->module_descriptor->init_arrays[i].end-ctx->module_descriptor->init_arrays[i].start;j+=sizeof(void*)){
			void* func=*((void*const*)(ctx->module_descriptor->init_arrays[i].start+j));
			if (func){
				((void (*)(void))func)();
				if (ctx->module->flags&_MODULE_FLAG_EARLY_UNLOAD){
					goto _early_unload;
				}
			}
		}
	}
	INFO("Adjusting sections...");
	_unmap_region(ctx,&(ctx->elf_region_ue));
	_unmap_region(ctx,&(ctx->elf_region_ur));
	_unmap_region(ctx,&(ctx->elf_region_uw));
	_adjust_region_flags(ctx,&(ctx->elf_region_iw));
	ctx->module->state=MODULE_STATE_LOADED;
	LOG("Module '%s' loaded successfully at %p",ctx->module->name->data,ctx->module->region->rb_node.key);
	event_dispatch(ctx->module->load_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	handle_release(&(ctx->module->handle));
	amm_dealloc(ctx);
	return;
_load_error:
	symbol_remove(ctx->name);
	ctx->module->state=MODULE_STATE_UNLOADED;
	if (ctx->module->region){
		mmap_dealloc_region(_module_image_mmap,ctx->module->region);
	}
	mmap_dealloc(process_kernel->mmap,(u64)(ctx->data),0);
	if (handle_release(&(ctx->module->handle))/* module self-handle */){
		handle_release(&(ctx->module->handle)); /* initializer thread handle */
	}
	amm_dealloc(ctx);
	return;
_early_unload:
	exception_unwind_pop();
	ctx->module->state=MODULE_STATE_LOADED;
	module_unload(ctx->module);
	event_dispatch(ctx->module->load_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	handle_release(&(ctx->module->handle));
	amm_dealloc(ctx);
}



KERNEL_EARLY_INIT(){
	LOG("Initializing module loader...");
	_module_allocator=omm_init("kernel.module",sizeof(module_t),8,4);
	rwlock_init(&(_module_allocator->lock));
	module_handle_type=handle_alloc("kernel.module",0,_module_handle_destructor);
	_module_image_mmap=mmap_init(&vmm_kernel_pagemap,aslr_module_base,aslr_module_base+aslr_module_size);
	_module_load_lock=mutex_create("kernel.module.load.lock");
	aslr_module_base=0;
	module_notification_dispatcher=notification_dispatcher_create("kernel.module");
}



KERNEL_PUBLIC KERNEL_AWAITS module_t* module_load(const char* name,bool async){
	if (!name){
		return NULL;
	}
	LOG("Loading module '%s'...",name);
	mutex_acquire(_module_load_lock);
	module_t* module=module_lookup(name);
	if (module){
		mutex_release(_module_load_lock);
		INFO("Module '%s' is already loaded",name);
		return module;
	}
	vfs_node_t* directory=vfs_lookup(NULL,MODULE_ROOT_DIRECTORY,0,0,0);
	// ================> exception unwinding <================
	// vfs_node_unref(directory)
	// smm_dealloc(name_string)
	// symbol_remove(name);
	// if (module->region){
	// 	mmap_dealloc_region(_module_image_mmap,module->region);
	// }
	// if (handle_release(&(module->handle))){
	// 	handle_release(&(module->handle));
	// }
	// mmap_dealloc_region(process_kernel->mmap,region);
	// ================> exception unwinding <================
	if (!directory){
		panic("Unable to find module root directory");
	}
	char buffer[256];
	string_t* name_string=smm_alloc(buffer,format_string(buffer,256,"%s.mod",name));
	vfs_node_t* module_file=vfs_node_lookup(directory,name_string);
	smm_dealloc(name_string);
	vfs_node_unref(directory);
#ifdef KERNEL_COVERAGE
	if (!module_file&&name[0]=='/'){
		module_file=vfs_lookup(NULL,name,0,0,0);
		for (const char* path=name;path[0];path++){
			if (path[0]=='/'){
				name=path+1;
			}
		}
	}
#endif
	if (!module_file){
		mutex_release(_module_load_lock);
		ERROR("Unable to find module '%s'",name);
		return NULL;
	}
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,0,MMAP_REGION_FLAG_NO_WRITEBACK|MMAP_REGION_FLAG_VMM_WRITE,module_file);
	vfs_node_unref(module_file);
	INFO("Module file size: %v",region->length);
	module=omm_alloc(_module_allocator);
	handle_new(module_handle_type,&(module->handle)); /* user handle */
	module->name=smm_alloc(name,0);
	module->region=NULL;
	module->flags=0;
	module->state=MODULE_STATE_LOADING;
	module->load_event=event_create("kernel.module.load",NULL);
	mutex_release(_module_load_lock);
	module_loader_context_t* ctx=amm_alloc(sizeof(module_loader_context_t));
	ctx->name=name;
	ctx->module=module;
	ctx->module_descriptor=NULL;
	ctx->data=(void*)(region->rb_node.key);
	_find_static_elf_sections(ctx);
	bool is_tainted=1;
	if (!_check_elf_header(ctx)||(!(module->flags&MODULE_FLAG_NO_SIGNATURE)&&!signature_verify_module(name,region,&is_tainted))){
		handle_release(&(module->handle));
		mmap_dealloc_region(process_kernel->mmap,region);
		amm_dealloc(ctx);
		return NULL;
	}
	if (is_tainted){
		module->flags|=MODULE_FLAG_TAINTED;
	}
	handle_acquire(&(module->handle)); /* module self-handle */
	handle_acquire(&(module->handle)); /* initializer thread handle */
	format_string(buffer,sizeof(buffer),"kernel.module.%s.init",name);
	thread_create_kernel_thread(NULL,buffer,_async_initialization_thread,1,ctx);
	if (async){
		event_await(&(module->load_event),1,0);
	}
	return module;
}



KERNEL_PUBLIC bool module_unload(module_t* module){
	if (module->state==MODULE_STATE_UNLOADING||module->state==MODULE_STATE_UNLOADED){
		return 0;
	}
	if (module->state==MODULE_STATE_LOADING){
		module->flags|=_MODULE_FLAG_EARLY_UNLOAD;
		return 1;
	}
	LOG("Unloading module '%s'...",module->name->data);
	_send_unload_notification(module);
	module->state=MODULE_STATE_UNLOADING;
	for (u64 i=0;i+sizeof(void*)<=module->deinit_array_size;i+=sizeof(void*)){
		void* func=*((void*const*)(module->deinit_array_base+i));
		if (func){
			((void (*)(void))func)();
		}
	}
	module->state=MODULE_STATE_UNLOADED;
	symbol_remove(module->name->data);
	if (module->region){
		mmap_dealloc_region(_module_image_mmap,module->region);
	}
	if (module->flags&MODULE_FLAG_PREVENT_LOADS){
		INFO("Preventing future module loads...");
		return 1;
	}
	handle_release(&(module->handle));
	return 1;
}



KERNEL_PUBLIC module_t* module_lookup(const char* name){
	HANDLE_FOREACH(module_handle_type){
		module_t* module=KERNEL_CONTAINEROF(handle,module_t,handle);
		if (str_equal(module->name->data,name)){
			return module;
		}
	}
	return NULL;
}
