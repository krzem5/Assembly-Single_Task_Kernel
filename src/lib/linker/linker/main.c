#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/elf/elf.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



extern const elf_dyn_t _DYNAMIC[];
extern u64 __gcov_info_start[1];
extern u64 __gcov_info_end[1];

linker_shared_object_t* linker_shared_object_executable=NULL;



u64 main(const u64* data){
	const char* path=(data[0]?(const char*)(data[1]):"???");
	u64 interpreter_image_base=0;
	u64 entry_address=0;
	const void* phdr_entries=NULL;
	u64 phdr_entry_size=0;
	u64 phdr_entry_count=0;
	for (data+=data[0]+1;data[0];data++);
	for (data++;data[0];data+=2){
		if (data[0]==AT_PHDR){
			phdr_entries=(void*)(data[1]);
		}
		else if (data[0]==AT_PHENT){
			phdr_entry_size=data[1];
		}
		else if (data[0]==AT_PHNUM){
			phdr_entry_count=data[1];
		}
		else if (data[0]==AT_BASE){
			interpreter_image_base=data[1];
		}
		else if (data[0]==AT_ENTRY){
			entry_address=data[1];
		}
	}
	const elf_dyn_t* dynamic_section=NULL;
	const char* interpreter="???";
	for (u16 i=0;i<phdr_entry_count;i++){
		const elf_phdr_t* program_header=phdr_entries+i*phdr_entry_size;
		if (program_header->p_type==PT_DYNAMIC){
			dynamic_section=(void*)(program_header->p_vaddr);
		}
		else if (program_header->p_type==PT_INTERP){
			interpreter=(void*)(program_header->p_vaddr);
		}
	}
	linker_shared_object_t* so=linker_shared_object_init(interpreter_image_base,_DYNAMIC,interpreter,0);
#ifdef KERNEL_COVERAGE
	so->gcov_info_base=(u64)__gcov_info_start;
	so->gcov_info_size=((u64)__gcov_info_end)-((u64)__gcov_info_start);
#else
	(void)so;
#endif
	linker_shared_object_executable=linker_shared_object_init(0,dynamic_section,path,0);
	return entry_address;
}
