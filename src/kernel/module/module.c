#include <kernel/aslr/aslr.h>
#include <kernel/elf/structures.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
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

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE module_handle_type=0;
KERNEL_PUBLIC notification_dispatcher_t* KERNEL_INIT_WRITE module_notification_dispatcher=NULL;



static void _module_handle_destructor(handle_t* handle){
	module_t* module=KERNEL_CONTAINEROF(handle,module_t,handle);
	smm_dealloc(module->name);
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



static bool _map_sections(module_loader_context_t* ctx){
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
			if (str_equal(name,"__module_header")){
				ctx->module_descriptor=(void*)address;
			}
			else if (!str_startswith(name,"__module")){
				symbol_add(ctx->module->name->data,name,address,(elf_symbol->st_info>>4)==STB_GLOBAL&&elf_symbol->st_other==STV_DEFAULT);
			}
		}
	}
	if (ret&&!ctx->module_descriptor){
		ERROR("module header not present");
		return 0;
	}
	return ret;
}



static bool _apply_relocations(module_loader_context_t* ctx){
	INFO("Applying relocations...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!section_header->sh_info||section_header->sh_info>=ctx->elf_header->e_shnum){
			continue;
		}
		u64 base=((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+section_header->sh_info*ctx->elf_header->e_shentsize))->sh_addr;
		if (!base||section_header->sh_type!=SHT_RELA){
			continue;
		}
		const elf_rela_t* entry=ctx->data+section_header->sh_offset;
		for (u64 j=0;j<section_header->sh_size;j+=sizeof(elf_rela_t)){
			const elf_sym_t* symbol=ctx->elf_symbol_table+(entry->r_info>>32);
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
	ctx->module->deinit_array_base=ctx->module_descriptor->deinit_start;
	ctx->module->deinit_array_size=ctx->module_descriptor->deinit_end-ctx->module_descriptor->deinit_start;
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



static bool _execute_initializers(module_loader_context_t* ctx){
	INFO("Executing initializers...");
	for (u64 i=0;i+sizeof(void*)<=ctx->module_descriptor->preinit_end-ctx->module_descriptor->preinit_start;i+=sizeof(void*)){
		void* func=*((void*const*)(ctx->module_descriptor->preinit_start+i));
		if (func&&!((bool (*)(void))func)()){
			return 0;
		}
	}
	for (u64 i=0;i+sizeof(void*)<=ctx->module_descriptor->init_end-ctx->module_descriptor->init_start;i+=sizeof(void*)){
		void* func=*((void*const*)(ctx->module_descriptor->init_start+i));
		if (func){
			((void (*)(void))func)();
		}
	}
	for (u64 i=0;i+sizeof(void*)<=ctx->module_descriptor->postinit_end-ctx->module_descriptor->postinit_start;i+=sizeof(void*)){
		void* func=*((void*const*)(ctx->module_descriptor->postinit_start+i));
		if (func){
			((void (*)(void))func)();
		}
	}
	for (u64 i=0;i+sizeof(void*)<=ctx->module_descriptor->postpostinit_end-ctx->module_descriptor->postpostinit_start;i+=sizeof(void*)){
		void* func=*((void*const*)(ctx->module_descriptor->postpostinit_start+i));
		if (func){
			((void (*)(void))func)();
		}
	}
	return 1;
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



KERNEL_EARLY_INIT(){
	LOG("Initializing module loader...");
	_module_allocator=omm_init("kernel.module",sizeof(module_t),8,4);
	rwlock_init(&(_module_allocator->lock));
	module_handle_type=handle_alloc("kernel.module",0,_module_handle_destructor);
	_module_image_mmap=mmap_init(&vmm_kernel_pagemap,aslr_module_base,aslr_module_base+aslr_module_size);
	aslr_module_base=0;
	module_notification_dispatcher=notification_dispatcher_create();
}



KERNEL_PUBLIC module_t* module_load(const char* name){
	if (!name){
		return NULL;
	}
	LOG("Loading module '%s'...",name);
	module_t* module=module_lookup(name);
	if (module){
		INFO("Module '%s' is already loaded",name);
		return module;
	}
	vfs_node_t* directory=vfs_lookup(NULL,MODULE_ROOT_DIRECTORY,0,0,0);
	if (!directory){
		panic("Unable to find module root directory");
	}
	char buffer[256];
	SMM_TEMPORARY_STRING name_string=smm_alloc(buffer,format_string(buffer,256,"%s.mod",name));
	vfs_node_t* module_file=vfs_node_lookup(directory,name_string);
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
		ERROR("Unable to find module '%s'",name);
		return NULL;
	}
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,0,MMAP_REGION_FLAG_NO_WRITEBACK|MMAP_REGION_FLAG_VMM_WRITE,module_file);
	vfs_node_unref(module_file);
	INFO("Module file size: %v",region->length);
	module=omm_alloc(_module_allocator);
	handle_new(module_handle_type,&(module->handle));
	handle_acquire(&(module->handle));
	module->name=smm_alloc(name,0);
	module->region=NULL;
	module->flags=0;
	module->state=MODULE_STATE_LOADING;
	module_loader_context_t ctx={
		name,
		module,
		NULL,
		(void*)(region->rb_node.key)
	};
	_find_static_elf_sections(&ctx);
	if (!_check_elf_header(&ctx)){
		goto _error;
	}
	bool is_tainted=1;
	if (!(module->flags&MODULE_FLAG_NO_SIGNATURE)&&!signature_verify_module(name,region,&is_tainted)){
		goto _error;
	}
	if (is_tainted){
		module->flags|=MODULE_FLAG_TAINTED;
	}
	if (!_map_sections(&ctx)){
		goto _error;
	}
	if (!_find_elf_sections(&ctx)){
		goto _error;
	}
	if (!_resolve_symbol_table(&ctx)){
		goto _error;
	}
	if (!_apply_relocations(&ctx)){
		goto _error;
	}
	_adjust_memory_flags(&ctx);
	mmap_dealloc_region(process_kernel->mmap,region);
	_process_module_header(&ctx);
	_send_load_notification(module);
	if (!_execute_initializers(&ctx)){
		module_unload(module);
		return NULL;
	}
	INFO("Adjusting sections...");
	_unmap_region(&ctx,&(ctx.elf_region_ue));
	_unmap_region(&ctx,&(ctx.elf_region_ur));
	_unmap_region(&ctx,&(ctx.elf_region_uw));
	_adjust_region_flags(&ctx,&(ctx.elf_region_iw));
	module->state=MODULE_STATE_LOADED;
	LOG("Module '%s' loaded successfully at %p",name,module->region->rb_node.key);
	return module;
_error:
	symbol_remove(name);
	if (module->region){
		mmap_dealloc_region(_module_image_mmap,module->region);
	}
	if (handle_release(&(module->handle))){
		handle_release(&(module->handle));
	}
	mmap_dealloc_region(process_kernel->mmap,region);
	return NULL;
}



KERNEL_PUBLIC bool module_unload(module_t* module){
	if (module->state==MODULE_STATE_UNLOADING||module->state==MODULE_STATE_UNLOADED){
		return 0;
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
			handle_release(handle);
			return module;
		}
	}
	return NULL;
}
