#include <kernel/cpu/cpu.h>
#include <kernel/fs/fd.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>



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



typedef struct _ELF_PROGRAM_HEADER{
	u32 p_type;
	u32 p_flags;
	u64 p_offset;
	u64 p_vaddr;
	u64 p_paddr;
	u64 p_filesz;
	u64 p_memsz;
	u64 p_align;
} elf_program_header_t;



void* elf_load(const char* path){
	LOG("Loading ELF executable '%s'...",path);
	fs_node_t* node=fs_get_node(NULL,path);
	if (!node){
		ERROR("File '%s' not found",path);
		return NULL;
	}
	vmm_pagemap_t pagemap;
	vmm_pagemap_init(&pagemap);
	elf_header_t header;
	if (fs_read(node,0,&header,sizeof(elf_header_t))!=sizeof(elf_header_t)){
		goto _error;
	}
	if (header.signature!=0x464c457f||header.word_size!=2||header.endianess!=1||header.header_version!=1||header.abi!=0||header.e_type!=2||header.e_machine!=0x3e||header.e_version!=1){
		goto _error;
	}
	u64 highest_address=0;
	for (u16 i=0;i<header.e_phnum;i++){
		elf_program_header_t program_header;
		if (fs_read(node,header.e_phoff+i*sizeof(elf_program_header_t),&program_header,sizeof(elf_program_header_t))!=sizeof(elf_program_header_t)){
			goto _error;
		}
		if (program_header.p_type!=1){
			continue;
		}
		u64 flags=VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_PRESENT;
		if (program_header.p_flags&1){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		else{
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		if (program_header.p_flags&2){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		u64 offset=program_header.p_vaddr&(PAGE_SIZE-1);
		u64 page_count=pmm_align_up_address(program_header.p_memsz+offset)>>PAGE_SIZE_SHIFT;
		u64 pages=pmm_alloc(page_count);
		vmm_map_pages(&pagemap,pages,program_header.p_vaddr-offset,flags|VMM_MAP_WITH_COUNT,page_count);
		u64 end_address=program_header.p_vaddr-offset+(page_count<<PAGE_SIZE_SHIFT);
		if (end_address>highest_address){
			highest_address=end_address;
		}
		if (fs_read(node,program_header.p_offset,VMM_TRANSLATE_ADDRESS(pages)+offset,program_header.p_filesz)!=program_header.p_filesz){
			goto _error;
		}
	}
	u16 cpu_count=cpu_get_core_count();
	for (u16 i=0;i<cpu_count;i++){
		vmm_map_pages(&pagemap,cpu_get_stack(i),cpu_get_stack_top(i)-(USER_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT),VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT,USER_STACK_PAGE_COUNT);
	}
	vmm_pagemap_deinit(&vmm_user_pagemap);
	vmm_user_pagemap=pagemap;
	mmap_set_range(highest_address,cpu_get_stack_top(cpu_count));
	fd_clear();
	return (void*)(header.e_entry);
_error:
	ERROR("Unable to load ELF file");
	vmm_pagemap_deinit(&pagemap);
	return NULL;
}
