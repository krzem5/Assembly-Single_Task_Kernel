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
	module_t* module=handle->object;
	ERROR("Delete module: %s",module->descriptor->name);
	omm_dealloc(&_module_allocator,module);
});



static module_t* _lookup_module_by_name(const char* name){
	HANDLE_FOREACH(HANDLE_TYPE_MODULE){
		handle_acquire(handle);
		module_t* module=handle->object;
		if (streq(module->descriptor->name,name)){
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
	const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+ctx->elf_header->e_shstrndx*sizeof(elf_shdr_t);
	ctx->elf_string_table=ctx->data+section_header->sh_offset;
}



static _Bool _check_elf_header(module_loader_context_t* ctx){
	if (ctx->elf_header->e_ident.signature!=0x464c457f){
		WARN("ELF header error: e_ident.signature != 0x464c457f");
		return 0;
	}
	if (ctx->elf_header->e_ident.word_size!=2){
		WARN("ELF header error: e_ident.word_size != 2");
		return 0;
	}
	if (ctx->elf_header->e_ident.endianess!=1){
		WARN("ELF header error: e_ident.endianess != 1");
		return 0;
	}
	if (ctx->elf_header->e_ident.header_version!=1){
		WARN("ELF header error: e_ident.header_version != 1");
		return 0;
	}
	if (ctx->elf_header->e_ident.abi!=0){
		WARN("ELF header error: e_ident.abi != 0");
		return 0;
	}
	if (ctx->elf_header->e_type!=ET_REL){
		WARN("ELF header error: e_type != ET_REL");
		return 0;
	}
	if (ctx->elf_header->e_machine!=0x3e){
		WARN("ELF header error: machine != 0x3e");
		return 0;
	}
	if (ctx->elf_header->e_version!=1){
		WARN("ELF header error: version != 1");
		return 0;
	}
	return 1;
}



static void _accumulate_sections(module_loader_context_t* ctx){
	INFO("Accumulating sections...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*sizeof(elf_shdr_t);
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
				panic("Invalid section flag combination");
		}
		if (section_header->sh_addralign){
			*var+=(-(*var))&(section_header->sh_addralign-1);
		}
		*var+=section_header->sh_size;
	}
}



static void _alloc_region_memory(module_address_range_t* region){
	region->size=pmm_align_up_address((region->size?region->size:1));
	mmap_region_t* mmap_region=mmap_alloc(&process_kernel_image_mmap,0,region->size,&_module_image_pmm_counter,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
	if (!mmap_region){
		panic("Unable to reserve module section memory");
	}
	region->base=mmap_region->rb_node.key;
}



static void _map_section(module_loader_context_t* ctx){
	INFO("Mapping sections...");
	_alloc_region_memory(&(ctx->module->ex_region));
	_alloc_region_memory(&(ctx->module->nx_region));
	_alloc_region_memory(&(ctx->module->rw_region));
	INFO("Regions: EX: %v, NX: %v, RW: %v",ctx->module->ex_region.size,ctx->module->nx_region.size,ctx->module->rw_region.size);
	u64 ex_base=ctx->module->ex_region.base;
	u64 nx_base=ctx->module->nx_region.base;
	u64 rw_base=ctx->module->rw_region.base;
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*sizeof(elf_shdr_t);
		if (!(section_header->sh_flags&SHF_ALLOC)){
			section_header->sh_addr=0;
			continue;
		}
		u64* var=NULL;
		switch (section_header->sh_flags&(SHF_WRITE|SHF_EXECINSTR)){
			case 0:
				var=&(nx_base);
				break;
			case SHF_WRITE:
				var=&(rw_base);
				break;
			case SHF_EXECINSTR:
				var=&(ex_base);
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
}



static void _find_dynamic_elf_sections(module_loader_context_t* ctx){
	INFO("Locating dynamic ELF sections...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*sizeof(elf_shdr_t);
		if (section_header->sh_type==SHT_SYMTAB){
			ctx->elf_symbol_table=ctx->data+section_header->sh_offset;
			ctx->elf_symbol_table_size=section_header->sh_size;
			section_header=ctx->data+ctx->elf_header->e_shoff+section_header->sh_link*sizeof(elf_shdr_t);
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
		panic("'.module' section not present");
	}
}



static void _resolve_symbol_table(module_loader_context_t* ctx){
	INFO("Resolving symbol table...");
	_Bool unresolved_symbols=0;
	for (u64 i=0;i<ctx->elf_symbol_table_size/sizeof(elf_sym_t);i++){
		elf_sym_t* elf_symbol=ctx->elf_symbol_table+i;
		if (elf_symbol->st_shndx==SHN_UNDEF&&ctx->elf_symbol_string_table[elf_symbol->st_name]){
			const symbol_t* symbol=symbol_lookup_by_name(ctx->elf_symbol_string_table+elf_symbol->st_name);
			if (symbol){
				elf_symbol->st_value=symbol->rb_node.key;
			}
			else{
				ERROR("Unresolved symbol: %s",ctx->elf_symbol_string_table+elf_symbol->st_name);
				unresolved_symbols=1;
			}
		}
		else if (elf_symbol->st_value){
			symbol_add(ctx->name,ctx->elf_symbol_string_table+elf_symbol->st_name,elf_symbol->st_value+((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+elf_symbol->st_shndx*sizeof(elf_shdr_t)))->sh_addr);
		}
	}
	if (unresolved_symbols){
		panic("unresolved module symbols");
	}
}



static void _apply_relocations(module_loader_context_t* ctx){
	INFO("Applying relocations...");
	for (u16 i=0;i<ctx->elf_header->e_shnum;i++){
		const elf_shdr_t* section_header=ctx->data+ctx->elf_header->e_shoff+i*sizeof(elf_shdr_t);
		if (!section_header->sh_info||section_header->sh_info>=ctx->elf_header->e_shnum){
			continue;
		}
		u64 base=((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+section_header->sh_info*sizeof(elf_shdr_t)))->sh_addr;
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
				u64 value=symbol->st_value+entry->r_addend+((const elf_shdr_t*)(ctx->data+ctx->elf_header->e_shoff+symbol->st_shndx*sizeof(elf_shdr_t)))->sh_addr;
				switch (entry->r_info&0xffffffff){
					case R_X86_64_NONE:
						break;
					case R_X86_64_64:
						*((u64*)relocation_address)=value;
						break;
					case R_X86_64_PC32:
					case R_X86_64_PLT32:
						*((u32*)relocation_address)=value-relocation_address;
						break;
					case R_X86_64_32:
					case R_X86_64_32S:
						*((u32*)relocation_address)=value;
						break;
					default:
						WARN("Unknown relocation type: %u",entry->r_info&0xffffffff);
						panic("Unknown relocation type");
				}
				entry++;
			}
		}
	}
}



static void _adjust_memory_flags(module_loader_context_t* ctx){
	INFO("Adjusting memory flags...");
	vmm_adjust_flags(&vmm_kernel_pagemap,ctx->module->ex_region.base,0,VMM_PAGE_FLAG_READWRITE,ctx->module->ex_region.size>>PAGE_SIZE_SHIFT);
	vmm_adjust_flags(&vmm_kernel_pagemap,ctx->module->nx_region.base,VMM_PAGE_FLAG_NOEXECUTE,VMM_PAGE_FLAG_READWRITE,ctx->module->nx_region.size>>PAGE_SIZE_SHIFT);
	vmm_adjust_flags(&vmm_kernel_pagemap,ctx->module->rw_region.base,VMM_PAGE_FLAG_NOEXECUTE,0,ctx->module->rw_region.size>>PAGE_SIZE_SHIFT);
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
	vfs_node_t* directory=vfs_lookup(NULL,MODULE_ROOT_DIRECTORY,0);
	if (!directory){
		panic("Unable to find module root directory");
	}
	char buffer[256];
	SMM_TEMPORARY_STRING name_string=smm_alloc(buffer,format_string(buffer,256,"%s.mod",name));
	vfs_node_t* module_file=vfs_node_lookup(directory,name_string);
	if (!module_file){
		WARN("Unable to find module '%s'",name);
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
	_accumulate_sections(&ctx);
	_map_section(&ctx);
	_find_dynamic_elf_sections(&ctx);
	_resolve_symbol_table(&ctx);
	_apply_relocations(&ctx);
	_adjust_memory_flags(&ctx);
	mmap_dealloc_region(&(process_kernel->mmap),region);
	LOG("Module '%s' loaded successfully",name);
	spinlock_release_exclusive(&_module_global_lock);
	handle_finish_setup(&(module->handle));
	module->descriptor->init_callback(module);
	module->state=MODULE_STATE_LOADED;
	return module;
_error:
	handle_release(&(module->handle));
	mmap_dealloc_region(&(process_kernel->mmap),region);
	spinlock_release_exclusive(&_module_global_lock);
	return NULL;
}
