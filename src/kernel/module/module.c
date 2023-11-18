#include <kernel/elf/structures.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/symbol/symbol.h>
#include <kernel/types.h>
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



static pmm_counter_descriptor_t _module_image_pmm_counter=PMM_COUNTER_INIT_STRUCT("module_image");
static pmm_counter_descriptor_t _module_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_module");
static omm_allocator_t _module_allocator=OMM_ALLOCATOR_INIT_STRUCT("module",sizeof(module_t),8,4,&_module_omm_pmm_counter);

static spinlock_t _module_global_lock=SPINLOCK_INIT_STRUCT;



HANDLE_DECLARE_TYPE(MODULE,{
	omm_dealloc(&_module_allocator,handle->object);
});



static _Bool _alloc_region_memory(module_address_range_t* region){
	region->size=pmm_align_up_address((region->size?region->size:1));
	mmap_region_t* mmap_region=mmap_alloc(&process_kernel_image_mmap,0,region->size,&_module_image_pmm_counter,MMAP_REGION_FLAG_COMMIT|VMM_PAGE_FLAG_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
	if (!mmap_region){
		ERROR("Unable to reserve module section memory");
		return 0;
	}
	region->base=mmap_region->rb_node.key;
	return 1;
}



static void _dealloc_region_memory(const module_address_range_t* region){
	if (!region->size||!region->base){
		return;
	}
	mmap_dealloc(&process_kernel_image_mmap,region->base,region->size);
}



static module_t* _lookup_module_by_name(const char* name){
	HANDLE_FOREACH(HANDLE_TYPE_MODULE){
		handle_acquire(handle);
		module_t* module=handle->object;
		if (streq(module->name->data,name)){
			handle_release(handle);
			return module;
		}
		handle_release(handle);
	}
	return NULL;
}



static void _find_static_elf_sections(module_loader_context_t* ctx){
	INFO("Locating static ELF sections...");
	ctx->elf_header=ctx->data;
	const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+ctx->elf_header->e_shstrndx*ctx->elf_header->e_shentsize;
	ctx->elf_string_table=ctx->data+section_header->sh_offset;
}



static _Bool _check_elf_header(module_loader_context_t* ctx){
	if (ctx->elf_header->e_ident.signature!=0x464c457f){
		ERROR("ELF header error: e_ident.signature != 0x464c457f");
		return 0;
	}
	if (ctx->elf_header->e_ident.word_size!=2){
		ERROR("ELF header error: e_ident.word_size != 2");
		return 0;
	}
	if (ctx->elf_header->e_ident.endianess!=1){
		ERROR("ELF header error: e_ident.endianess != 1");
		return 0;
	}
	if (ctx->elf_header->e_ident.header_version!=1){
		ERROR("ELF header error: e_ident.header_version != 1");
		return 0;
	}
	if (ctx->elf_header->e_ident.abi!=0){
		ERROR("ELF header error: e_ident.abi != 0");
		return 0;
	}
	if (ctx->elf_header->e_type!=ET_REL){
		ERROR("ELF header error: e_type != ET_REL");
		return 0;
	}
	if (ctx->elf_header->e_machine!=0x3e){
		ERROR("ELF header error: machine != 0x3e");
		return 0;
	}
	if (ctx->elf_header->e_version!=1){
		ERROR("ELF header error: version != 1");
		return 0;
	}
	return 1;
}



static _Bool _accumulate_sections(module_loader_context_t* ctx){
	INFO("Accumulating sections...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!(section_header->sh_flags&SHF_ALLOC)){
			continue;
		}
		u64* var=NULL;
		switch (section_header->sh_flags&(SHF_WRITE|SHF_EXECINSTR)){
			case 0:
				var=&(ctx->module->nx_region.size);
				break;
			case SHF_WRITE:
				var=&(ctx->module->rw_region.size);
				break;
			case SHF_EXECINSTR:
				var=&(ctx->module->ex_region.size);
				break;
			default:
				ERROR("Invalid section flag combination: SHF_WRITE|SHF_EXECINSTR");
				return 0;
		}
		if (section_header->sh_addralign){
			*var+=(-(*var))&(section_header->sh_addralign-1);
		}
		*var+=section_header->sh_size;
	}
	return 1;
}



static _Bool _map_sections(module_loader_context_t* ctx){
	INFO("Mapping sections...");
	if (!_alloc_region_memory(&(ctx->module->ex_region))){
		return 0;
	}
	if (!_alloc_region_memory(&(ctx->module->nx_region))){
		return 0;
	}
	if (!_alloc_region_memory(&(ctx->module->rw_region))){
		return 0;
	}
	INFO("Regions: EX: %v, NX: %v, RW: %v",ctx->module->ex_region.size,ctx->module->nx_region.size,ctx->module->rw_region.size);
	u64 ex_base=ctx->module->ex_region.base;
	u64 nx_base=ctx->module->nx_region.base;
	u64 rw_base=ctx->module->rw_region.base;
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (!(section_header->sh_flags&SHF_ALLOC)){
			section_header->sh_addr=0;
			continue;
		}
		u64* var=NULL;
		switch (section_header->sh_flags&(SHF_WRITE|SHF_EXECINSTR)){
			case 0:
				var=&nx_base;
				break;
			case SHF_WRITE:
				var=&rw_base;
				break;
			case SHF_EXECINSTR:
				var=&ex_base;
				break;
		}
		if (section_header->sh_addralign){
			*var+=(-*var)&(section_header->sh_addralign-1);
		}
		section_header->sh_addr=*var;
		*var+=section_header->sh_size;
		if (section_header->sh_type==SHT_PROGBITS){
			memcpy((void*)(section_header->sh_addr),ctx->data+section_header->sh_offset,section_header->sh_size);
		}
	}
	return 1;
}



static _Bool _find_elf_sections(module_loader_context_t* ctx){
	INFO("Locating ELF sections...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*ctx->elf_header->e_shentsize;
		if (section_header->sh_type==SHT_SYMTAB){
			ctx->elf_symbol_table=ctx->data+section_header->sh_offset;
			ctx->elf_symbol_table_size=section_header->sh_size;
			section_header=ctx->data+ctx->elf_header->e_shoff+section_header->sh_link*ctx->elf_header->e_shentsize;
			ctx->elf_symbol_string_table=ctx->data+section_header->sh_offset;
		}
		else if (streq(ctx->elf_string_table+section_header->sh_name,".module")){
			ctx->module->descriptor=(void*)(section_header->sh_addr);
		}
		else if (streq(ctx->elf_string_table+section_header->sh_name,".gcov_info")){
			ctx->module->gcov_info.base=section_header->sh_addr;
			ctx->module->gcov_info.size=section_header->sh_size;
			INFO("Found .gcov_info section at %p (%v)",ctx->module->gcov_info.base,ctx->module->gcov_info.size);
		}
	}
	if (!ctx->module->descriptor){
		ERROR("'.module' section not present");
		return 0;
	}
	return 1;
}



static _Bool _resolve_symbol_table(module_loader_context_t* ctx){
	INFO("Resolving symbol table...");
	_Bool ret=1;
	for (u64 i=0;i<ctx->elf_symbol_table_size/sizeof(elf_sym_t);i++){
		elf_sym_t* elf_symbol=ctx->elf_symbol_table+i;
		if (elf_symbol->st_shndx==SHN_UNDEF&&ctx->elf_symbol_string_table[elf_symbol->st_name]){
			const symbol_t* symbol=symbol_lookup_by_name(ctx->elf_symbol_string_table+elf_symbol->st_name);
			if (symbol){
				elf_symbol->st_value=symbol->rb_node.key;
			}
			else{
				ERROR("Unresolved symbol: %s",ctx->elf_symbol_string_table+elf_symbol->st_name);
				ret=0;
			}
		}
		else if (elf_symbol->st_value){
			symbol_add(ctx->name,ctx->elf_symbol_string_table+elf_symbol->st_name,elf_symbol->st_value+((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+elf_symbol->st_shndx*ctx->elf_header->e_shentsize))->sh_addr);
		}
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
		if (!base){
			continue;
		}
		if (section_header->sh_type==SHT_REL){
			panic("SHT_REL");
		}
		else if (section_header->sh_type==SHT_RELA){
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
	}
	return 1;
}



static void _adjust_memory_flags(module_loader_context_t* ctx){
	INFO("Adjusting memory flags...");
	vmm_adjust_flags(&vmm_kernel_pagemap,ctx->module->ex_region.base,0,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE,ctx->module->ex_region.size>>PAGE_SIZE_SHIFT,1);
	vmm_adjust_flags(&vmm_kernel_pagemap,ctx->module->nx_region.base,0,VMM_PAGE_FLAG_READWRITE,ctx->module->nx_region.size>>PAGE_SIZE_SHIFT,1);
	vmm_adjust_flags(&vmm_kernel_pagemap,ctx->module->rw_region.base,0,0,ctx->module->rw_region.size>>PAGE_SIZE_SHIFT,1);
}



module_t* module_load(const char* name){
	if (!name){
		return NULL;
	}
	LOG("Loading module '%s'...",name);
	spinlock_acquire_exclusive(&_module_global_lock);
	module_t* module=_lookup_module_by_name(name);
	if (module){
		INFO("Module '%s' already loaded",name);
		spinlock_release_exclusive(&_module_global_lock);
		return module;
	}
	vfs_node_t* directory=vfs_lookup(NULL,MODULE_ROOT_DIRECTORY,0,0,0);
	if (!directory){
		panic("Unable to find module root directory");
	}
	char buffer[256];
	SMM_TEMPORARY_STRING name_string=smm_alloc(buffer,format_string(buffer,256,"%s.mod",name));
	vfs_node_t* module_file=vfs_node_lookup(directory,name_string);
	if (!module_file){
		ERROR("Unable to find module '%s'",name);
		spinlock_release_exclusive(&_module_global_lock);
		return NULL;
	}
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,0,NULL,MMAP_REGION_FLAG_NO_FILE_WRITEBACK|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,module_file);
	INFO("Module file size: %v",region->length);
	module=omm_alloc(&_module_allocator);
	memset(module,0,sizeof(module_t));
	module->state=MODULE_STATE_LOADING;
	handle_new(module,HANDLE_TYPE_MODULE,&(module->handle));
	module_loader_context_t ctx={
		name,
		module,
		(void*)(region->rb_node.key)
	};
	_find_static_elf_sections(&ctx);
	if (!_check_elf_header(&ctx)){
		goto _error;
	}
	if (!_accumulate_sections(&ctx)){
		goto _error;
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
	mmap_dealloc_region(&(process_kernel->mmap),region);
	LOG("Module '%s' loaded successfully",name);
	spinlock_release_exclusive(&_module_global_lock);
	module->name=smm_alloc(name,0);
	handle_finish_setup(&(module->handle));
	module->flags=module->descriptor->flags;
	*(module->descriptor->module_self_ptr)=module;
	if (!module->descriptor->init_callback(module)){
		module_unload(module);
		return NULL;
	}
	module->state=MODULE_STATE_LOADED;
	return module;
_error:
	_dealloc_region_memory(&(module->ex_region));
	_dealloc_region_memory(&(module->nx_region));
	_dealloc_region_memory(&(module->rw_region));
	handle_release(&(module->handle));
	mmap_dealloc_region(&(process_kernel->mmap),region);
	spinlock_release_exclusive(&_module_global_lock);
	return NULL;
}



void module_unload(module_t* module){
	if (module->state==MODULE_STATE_UNLOADING||module->state==MODULE_STATE_UNLOADED){
		return;
	}
	module->state=MODULE_STATE_UNLOADING;
	module->descriptor->deinit_callback(module);
	module->state=MODULE_STATE_UNLOADED;
	_dealloc_region_memory(&(module->ex_region));
	_dealloc_region_memory(&(module->nx_region));
	_dealloc_region_memory(&(module->rw_region));
	if (module->flags&MODULE_FLAG_PREVENT_LOADS){
		return;
	}
	spinlock_acquire_exclusive(&_module_global_lock);
	smm_dealloc(module->name);
	handle_release(&(module->handle));
	spinlock_release_exclusive(&_module_global_lock);
}
