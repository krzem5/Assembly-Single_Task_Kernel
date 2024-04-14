#include <kernel/aslr/aslr.h>
#include <kernel/elf/structures.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
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



typedef struct _MODULE_LOADER_CONTEXT{
	const char* name;
	module_t* module;
	void* data;
	const elf_hdr_t* elf_header;
	const char* elf_string_table;
	elf_sym_t* elf_symbol_table;
	u64 elf_symbol_table_size;
	const char* elf_symbol_string_table;
} module_loader_context_t;



static omm_allocator_t* _module_allocator=NULL;
static mmap_t* _module_image_mmap=NULL;

KERNEL_PUBLIC handle_type_t module_handle_type=0;



static void _module_handle_destructor(handle_t* handle){
	module_t* module=handle->object;
	smm_dealloc(module->name);
	omm_dealloc(_module_allocator,module);
}



static void _find_static_elf_sections(module_loader_context_t* ctx){
	INFO("Locating static ELF sections...");
	ctx->elf_header=ctx->data;
	const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+ctx->elf_header->e_shstrndx*ctx->elf_header->e_shentsize;
	ctx->elf_string_table=ctx->data+section_header->sh_offset;
}



static _Bool _check_elf_header(module_loader_context_t* ctx){
	if (ctx->elf_header->e_ident.signature!=0x464c457f||ctx->elf_header->e_ident.word_size!=2||ctx->elf_header->e_ident.endianess!=1||ctx->elf_header->e_ident.header_version!=1||ctx->elf_header->e_ident.abi!=0||ctx->elf_header->e_type!=ET_REL||ctx->elf_header->e_machine!=0x3e||ctx->elf_header->e_version!=1){
		ERROR("Invalid ELF header");
		return 0;
	}
	return 1;
}



static _Bool _map_sections(module_loader_context_t* ctx){
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
		if (section_header->sh_type==SHT_PROGBITS){
			mem_copy((void*)(section_header->sh_addr),ctx->data+section_header->sh_offset,section_header->sh_size);
		}
	}
	return 1;
}



static _Bool _find_elf_sections(module_loader_context_t* ctx){
	INFO("Locating ELF sections...");
	ctx->elf_symbol_table=NULL;
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (section_header->sh_type==SHT_SYMTAB){
			ctx->elf_symbol_table=ctx->data+section_header->sh_offset;
			ctx->elf_symbol_table_size=section_header->sh_size;
			section_header=ctx->data+ctx->elf_header->e_shoff+section_header->sh_link*ctx->elf_header->e_shentsize;
			ctx->elf_symbol_string_table=ctx->data+section_header->sh_offset;
			break;
		}
	}
	if (!ctx->elf_symbol_table){
		ERROR("'.symtab' section not present");
		return 0;
	}
	return 1;
}



static _Bool _resolve_symbol_table(module_loader_context_t* ctx){
	INFO("Resolving symbol table...");
	_Bool ret=1;
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
				ctx->module->descriptor=(void*)address;
			}
			else if (!str_startswith(name,"__module")){
				symbol_add(ctx->module->name->data,name,address,(elf_symbol->st_info>>4)==STB_GLOBAL&&elf_symbol->st_other==STV_DEFAULT);
			}
		}
	}
	if (ret&&!ctx->module->descriptor){
		ERROR("module header not present");
		return 0;
	}
	return ret;
}



static _Bool _apply_relocations(module_loader_context_t* ctx){
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
		u64 addr=pmm_align_down_address(section_header->sh_addr);
		u64 flags=0;
		if (section_header->sh_flags&SHF_WRITE){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		if (!(section_header->sh_flags&SHF_EXECINSTR)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		vmm_adjust_flags(&vmm_kernel_pagemap,addr,flags,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,pmm_align_up_address(section_header->sh_size+section_header->sh_addr-addr)>>PAGE_SIZE_SHIFT,1);
	}
}



KERNEL_EARLY_INIT(){
	LOG("Initializing module loader...");
	_module_allocator=omm_init("module",sizeof(module_t),8,4,pmm_alloc_counter("omm_module"));
	spinlock_init(&(_module_allocator->lock));
	module_handle_type=handle_alloc("module",_module_handle_destructor);
	_module_image_mmap=mmap_init(&vmm_kernel_pagemap,aslr_module_base,aslr_module_base+aslr_module_size);
	aslr_module_base=0;
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
	INFO("Module file size: %v",region->length);
	module=omm_alloc(_module_allocator);
	handle_new(module,module_handle_type,&(module->handle));
	module->name=smm_alloc(name,0);
	module->descriptor=NULL;
	module->region=NULL;
	module->flags=0;
	module->state=MODULE_STATE_LOADING;
	module_loader_context_t ctx={
		name,
		module,
		(void*)(region->rb_node.key)
	};
	_find_static_elf_sections(&ctx);
	if (!_check_elf_header(&ctx)){
		goto _error;
	}
	_Bool is_tainted=1;
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
	module->deinit_array_base=module->descriptor->deinit_start;
	module->deinit_array_size=module->descriptor->deinit_end-module->descriptor->deinit_start;
#ifdef KERNEL_COVERAGE
	module->gcov_info_base=module->descriptor->gcov_info_start;
	module->gcov_info_size=module->descriptor->gcov_info_end-module->descriptor->gcov_info_start;
	if (module->gcov_info_size){
		INFO("Found .gcov_info data at %p (%u file%s)",module->gcov_info_base,module->gcov_info_size/sizeof(void*),(module->gcov_info_size/sizeof(void*)==1?"":"s"));
	}
	else{
		module->gcov_info_base=0;
	}
#endif
	mmap_dealloc_region(process_kernel->mmap,region);
	LOG("Module '%s' loaded successfully at %p",name,module->region->rb_node.key);
	handle_finish_setup(&(module->handle));
	module->flags=module->descriptor->flags;
	*(module->descriptor->module_self_ptr)=module;
	for (u64 i=0;i+sizeof(void*)<=module->descriptor->preinit_end-module->descriptor->preinit_start;i+=sizeof(void*)){
		void* func=*((void*const*)(module->descriptor->preinit_start+i));
		if (func&&!((_Bool (*)(void))func)()){
			module_unload(module);
			return NULL;
		}
	}
	for (u64 i=0;i+sizeof(void*)<=module->descriptor->init_end-module->descriptor->init_start;i+=sizeof(void*)){
		void* func=*((void*const*)(module->descriptor->init_start+i));
		if (func){
			((void (*)(void))func)();
		}
	}
	for (u64 i=0;i+sizeof(void*)<=module->descriptor->postinit_end-module->descriptor->postinit_start;i+=sizeof(void*)){
		void* func=*((void*const*)(module->descriptor->postinit_start+i));
		if (func){
			((void (*)(void))func)();
		}
	}
	module->state=MODULE_STATE_LOADED;
	return module;
_error:
	symbol_remove(name);
	if (module->region){
		mmap_dealloc_region(_module_image_mmap,module->region);
	}
	handle_release(&(module->handle));
	mmap_dealloc_region(process_kernel->mmap,region);
	return NULL;
}



KERNEL_PUBLIC _Bool module_unload(module_t* module){
	if (module->state==MODULE_STATE_UNLOADING||module->state==MODULE_STATE_UNLOADED){
		return 0;
	}
	LOG("Unloading module '%s'...",module->name->data);
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
		handle_acquire(handle);
		module_t* module=handle->object;
		if (str_equal(module->name->data,name)){
			handle_release(handle);
			return module;
		}
		handle_release(handle);
	}
	return NULL;
}
