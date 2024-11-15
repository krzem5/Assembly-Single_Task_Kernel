#include <common/types.h>



#define DT_NULL 0
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9

#define R_X86_64_RELATIVE 8



typedef struct _ELF_DYN{
	s64 d_tag;
	union{
		u64 d_val;
		void* d_ptr;
	} d_un;
} elf_dyn_t;



typedef struct _ELF_RELA{
	u64 r_offset;
	u64 r_info;
	s64 r_addend;
} elf_rela_t;



extern u64 _IMAGE_BASE;
extern elf_dyn_t* _DYNAMIC;



void relocate_executable(void){
	const elf_rela_t* relocations=NULL;
	u64 relocation_size=0;
	u64 relocation_entry_size=0;
	for (elf_dyn_t* dyn=_DYNAMIC;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_RELA:
				relocations=(const elf_rela_t*)(dyn->d_un.d_ptr+_IMAGE_BASE);
				break;
			case DT_RELASZ:
				relocation_size=dyn->d_un.d_val;
				break;
			case DT_RELAENT:
				relocation_entry_size=dyn->d_un.d_val;
				break;
		}
	}
	if (!relocations||!relocation_size||!relocation_entry_size){
		return;
	}
	while (1){
		if ((relocations->r_info&0xffffffff)==R_X86_64_RELATIVE){
			*((u64*)(_IMAGE_BASE+relocations->r_offset))+=_IMAGE_BASE;
		}
		if (relocation_size<=relocation_entry_size){
			return;
		}
		relocations=(const elf_rela_t*)(((u64)relocations)+relocation_entry_size);
		relocation_size-=relocation_entry_size;
	}
}
