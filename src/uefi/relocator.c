#include <common/types.h>
#include <elf.h>



extern u64 ImageBase;
extern Elf64_Dyn* _DYNAMIC;



void relocate_executable(void){
	Elf64_Rel* relocations=NULL;
	Elf64_Xword relocation_size=0;
	Elf64_Xword relocation_entry_size=0;
	for (Elf64_Dyn* dyn=_DYNAMIC;dyn->d_tag!=DT_NULL;dyn++){
		switch (dyn->d_tag){
			case DT_RELA:
				relocations=(Elf64_Rel*)(dyn->d_un.d_ptr+ImageBase);
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
		if (ELF64_R_TYPE(relocations->r_info)==R_X86_64_RELATIVE){
			*((u64*)(ImageBase+relocations->r_offset))+=ImageBase;
		}
		if (relocation_size<=relocation_entry_size){
			return;
		}
		relocations=(Elf64_Rel*)(((u64)relocations)+relocation_entry_size);
		relocation_size-=relocation_entry_size;
	}
}
