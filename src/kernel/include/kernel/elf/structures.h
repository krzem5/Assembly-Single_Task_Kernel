#ifndef _KERNEL_ELF_STRUCTUERS_H_
#define _KERNEL_ELF_STRUCTUERS_H_ 1
#include <kernel/types.h>



#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4
#define PT_SHLIB 5
#define PT_PHDR 6
#define PT_TLS 7

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

#define PF_R 4
#define PF_W 2
#define PF_X 1

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_NUM 12

#define SHF_WRITE 1
#define SHF_ALLOC 2
#define SHF_EXECINSTR 4

#define SHN_UNDEF 0

#define R_X86_64_NONE 0
#define R_X86_64_64 1
#define R_X86_64_PC32 2
#define R_X86_64_PLT32 4
#define R_X86_64_32 10
#define R_X86_64_32S 11



typedef struct _ELF_HDR{
	struct{
		u32 signature;
		u8 word_size;
		u8 endianess;
		u8 header_version;
		u8 abi;
		u8 _padding[8];
	} e_ident;
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
} elf_hdr_t;



typedef struct _ELF_PHDR{
	u32 p_type;
	u32 p_flags;
	u64 p_offset;
	u64 p_vaddr;
	u64 p_paddr;
	u64 p_filesz;
	u64 p_memsz;
	u64 p_align;
} elf_phdr_t;



typedef struct _ELF_SHDR{
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
} elf_shdr_t;



typedef struct _ELF_DYN{
	s64 d_tag;
	union{
		u64 d_val;
		u64 d_ptr;
	} d_un;
} elf_dyn_t;



typedef struct _ELF_REL{
	u64 r_offset;
	u64 r_info;
} elf_rel_t;



typedef struct _ELF_RELA{
	u64 r_offset;
	u64 r_info;
	s64 r_addend;
} elf_rela_t;



typedef struct _ELF_SYM{
	u32 st_name;
	u8 st_info;
	u8 st_other;
	u16 st_shndx;
	u64 st_value;
	u64 st_size;
} elf_sym_t;



#endif
