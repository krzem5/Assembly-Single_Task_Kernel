#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "module"



#define SHN_UNDEF 0

#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_NOBITS 8
#define SHT_REL 9

#define SHF_WRITE 1
#define SHF_ALLOC 2
#define SHF_EXECINSTR 4

#define R_X86_64_NONE 0
#define R_X86_64_64 1
#define R_X86_64_PC32 2
#define R_X86_64_PLT32 4
#define R_X86_64_32 10



typedef struct _ELF_HEADER{
	u32 signature;
	u8 word_size;
	u8 endianess;
	u8 header_version;
	u8 abi;
	u8 _padding[8];
	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u64 e_entry;
	u64 e_phoff;
	u64 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} elf_header_t;



typedef struct _ELF_SECTION_HEADER{
	u32 sh_name;
	u32 sh_type;
	u64 sh_flags;
	u64 sh_addr;
	u64 sh_offset;
	u64 sh_size;
	u32 sh_link;
	u32 sh_info;
	u64 sh_addralign;
	u64 sh_entsize;
} elf_section_header_t;



typedef struct _ELF_RELOCATION_ENTRY{
	u64 r_offset;
	u64 r_info;
} elf_relocation_entry_t;



typedef struct _ELF_RELOCATION_ADDEND_ENTRY{
	u64 r_offset;
	u64 r_info;
	s64 r_addend;
} elf_relocation_addend_entry_t;



typedef struct _ELF_SYMBOL_TABLE_ENTRY{
	u32 st_name;
	u8 st_info;
	u8 st_other;
	u16 st_shndx;
	u64 st_value;
	u64 st_size;
} elf_symbol_table_entry_t;



PMM_DECLARE_COUNTER(MODULE_BUFFER);
PMM_DECLARE_COUNTER(MODULE_IMAGE);
PMM_DECLARE_COUNTER(OMM_MODULE);



static omm_allocator_t _module_allocator=OMM_ALLOCATOR_INIT_STRUCT("module",sizeof(module_t),8,4,PMM_COUNTER_OMM_MODULE);



HANDLE_DECLARE_TYPE(MODULE,{
	module_t* module=handle->object;
	ERROR("Delete MODULE: %s",module->descriptor->name);
	omm_dealloc(&_module_allocator,module);
});



static void _module_alloc_region(module_address_region_t* region){
	region->size=pmm_align_up_address((region->size?region->size:1));
	u64 base=pmm_alloc(region->size>>PAGE_SIZE_SHIFT,PMM_COUNTER_MODULE_IMAGE,0);
	region->base=vmm_memory_map_reserve(&process_kernel_image_mmap,0,region->size);
	if (!region->base){
		panic("Unable to reserve module section memory");
	}
	vmm_map_pages(&vmm_kernel_pagemap,base,region->base,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT,region->size>>PAGE_SIZE_SHIFT);
}



static void _map_section_addresses(void* file_data,const elf_header_t* header,module_t* module){
	elf_section_header_t* section_header=file_data+header->e_shoff+header->e_shstrndx*sizeof(elf_section_header_t);
	const char* string_table=file_data+section_header->sh_offset;
	module->ex_region.size=0;
	module->nx_region.size=0;
	module->rw_region.size=0;
	section_header=file_data+header->e_shoff;
	for (u16 i=0;i<header->e_shnum;i++){
		if (!(section_header->sh_flags&SHF_ALLOC)){
			section_header++;
			continue;
		}
		u64* var=NULL;
		switch (section_header->sh_flags&(SHF_WRITE|SHF_EXECINSTR)){
			case 0:
				var=&(module->nx_region.size);
				break;
			case SHF_WRITE:
				var=&(module->rw_region.size);
				break;
			case SHF_EXECINSTR:
				var=&(module->ex_region.size);
				break;
			default:
				panic("Invalid section flag combination");
		}
		if (section_header->sh_addralign){
			*var+=(-*var)&(section_header->sh_addralign-1);
		}
		*var+=section_header->sh_size;
		section_header++;
	}
	_module_alloc_region(&(module->ex_region));
	_module_alloc_region(&(module->nx_region));
	_module_alloc_region(&(module->rw_region));
	section_header=file_data+header->e_shoff;
	u64 ex_base=module->ex_region.base;
	u64 nx_base=module->nx_region.base;
	u64 rw_base=module->rw_region.base;
	module->descriptor=NULL;
	for (u16 i=0;i<header->e_shnum;i++){
		if (!(section_header->sh_flags&SHF_ALLOC)){
			section_header->sh_addr=0;
			section_header++;
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
		if (streq(string_table+section_header->sh_name,".module")){
			module->descriptor=(void*)(section_header->sh_addr);
		}
		memcpy((void*)(section_header->sh_addr),file_data+section_header->sh_offset,section_header->sh_size);
		*var+=section_header->sh_size;
		section_header++;
	}
	if (!module->descriptor){
		panic("'.module' section not present");
	}
}



static void _resolve_symbol_table(void* file_data,const elf_header_t* header,elf_symbol_table_entry_t* symbol_table,u64 symbol_table_size,const char* string_table){
	for (u64 i=0;i<symbol_table_size;i+=sizeof(elf_symbol_table_entry_t)){
		if (symbol_table->st_shndx==SHN_UNDEF){
			symbol_table->st_value=kernel_lookup_symbol_address(string_table+symbol_table->st_name);
		}
		symbol_table->st_value+=((const elf_section_header_t*)(file_data+header->e_shoff+symbol_table->st_shndx*sizeof(elf_section_header_t)))->sh_addr;
		symbol_table++;
	}
}



static void _apply_relocations(void* file_data,const elf_header_t* header){
	const elf_section_header_t* section_header=file_data+header->e_shoff;
	elf_symbol_table_entry_t* symbol_table=NULL;
	u64 symbol_table_size=0;
	const char* string_table=NULL;
	for (u16 i=0;i<header->e_shnum;i++){
		if (section_header->sh_type==SHT_SYMTAB){
			symbol_table=file_data+section_header->sh_offset;
			symbol_table_size=section_header->sh_size;
			section_header=file_data+header->e_shoff+section_header->sh_link*sizeof(elf_section_header_t);
			string_table=file_data+section_header->sh_offset;
			break;
		}
		section_header++;
	}
	_resolve_symbol_table(file_data,header,symbol_table,symbol_table_size,string_table);
	section_header=file_data+header->e_shoff;
	for (u16 i=0;i<header->e_shnum;i++){
		u64 base=((const elf_section_header_t*)(file_data+header->e_shoff+section_header->sh_info*sizeof(elf_section_header_t)))->sh_addr;
		if (section_header->sh_type==SHT_REL&&base){
			panic("SHT_REL");
		}
		if (section_header->sh_type==SHT_RELA&&base){
			const elf_relocation_addend_entry_t* entry=file_data+section_header->sh_offset;
			for (u64 i=0;i<section_header->sh_size;i+=sizeof(elf_relocation_addend_entry_t)){
				const elf_symbol_table_entry_t* symbol=symbol_table+(entry->r_info>>32);
				u64 relocation_address=base+entry->r_offset;
				u64 value=symbol->st_value+entry->r_addend;
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
						*((u32*)relocation_address)=value;
						break;
					default:
						WARN("Unknon relocation type: %u",entry->r_info&0xffffffff);
						panic("Unknown relocation type");
				}
				entry++;
			}
		}
		section_header++;
	}
}



_Bool module_load(vfs_node_t* node){
	if (!node){
		return 0;
	}
	elf_header_t header;
	if (vfs_node_read(node,0,&header,sizeof(elf_header_t))!=sizeof(elf_header_t)){
		return 0;
	}
	if (header.signature!=0x464c457f||header.word_size!=2||header.endianess!=1||header.header_version!=1||header.abi!=0||header.e_type!=1||header.e_machine!=0x3e||header.e_version!=1){
		return 0;
	}
	u64 file_size=vfs_node_resize(node,0,VFS_NODE_FLAG_RESIZE_RELATIVE);
	u64 file_data_pages=pmm_align_up_address(file_size)>>PAGE_SIZE_SHIFT;
	void* file_data=(void*)(pmm_alloc(file_data_pages,PMM_COUNTER_MODULE_BUFFER,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	vfs_node_read(node,0,file_data,file_size);
	module_t* module=omm_alloc(&_module_allocator);
	handle_new(module,HANDLE_TYPE_MODULE,&(module->handle));
	_map_section_addresses(file_data,&header,module);
	_apply_relocations(file_data,&header);
	pmm_dealloc(((u64)file_data)-VMM_HIGHER_HALF_ADDRESS_OFFSET,file_data_pages,PMM_COUNTER_MODULE_BUFFER);
	vmm_adjust_flags(&vmm_kernel_pagemap,module->ex_region.base,0,VMM_PAGE_FLAG_READWRITE,module->ex_region.size>>PAGE_SIZE_SHIFT);
	vmm_adjust_flags(&vmm_kernel_pagemap,module->nx_region.base,VMM_PAGE_FLAG_NOEXECUTE,VMM_PAGE_FLAG_READWRITE,module->nx_region.size>>PAGE_SIZE_SHIFT);
	vmm_adjust_flags(&vmm_kernel_pagemap,module->rw_region.base,VMM_PAGE_FLAG_NOEXECUTE,0,module->rw_region.size>>PAGE_SIZE_SHIFT);
	LOG("Module '%s' loaded",module->descriptor->name);
	return module->descriptor->init_callback(module);
}
