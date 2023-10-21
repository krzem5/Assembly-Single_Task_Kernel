#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "module"



#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4



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
	for (u16 i=0;i<header.e_shnum;i++){
		elf_section_header_t section_header;
		if (vfs_node_read(node,header.e_shoff+i*sizeof(elf_section_header_t),&section_header,sizeof(elf_section_header_t))!=sizeof(elf_section_header_t)){
			return 0;
		}
		switch (section_header.sh_type){
			case SHT_PROGBITS:
				INFO("SHT_PROGBITS:\t%p\t%p\t%u\t%u",section_header.sh_offset,section_header.sh_size,section_header.sh_addralign,section_header.sh_entsize);
				break;
			case SHT_SYMTAB:
				INFO("SHT_SYMTAB:\t%p\t%p\t%u\t%u",section_header.sh_offset,section_header.sh_size,section_header.sh_addralign,section_header.sh_entsize);
				break;
			case SHT_STRTAB:
				INFO("SHT_STRTAB:\t%p\t%p\t%u\t%u",section_header.sh_offset,section_header.sh_size,section_header.sh_addralign,section_header.sh_entsize);
				break;
			case SHT_RELA:
				INFO("SHT_RELA:\t%p\t%p\t%u\t%u",section_header.sh_offset,section_header.sh_size,section_header.sh_addralign,section_header.sh_entsize);
				break;
		}
	}
	// extern void acpi_fadt_shutdown(_Bool restart);acpi_fadt_shutdown(0);
	return 0;
}
