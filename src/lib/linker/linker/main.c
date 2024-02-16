#include <linker/alloc.h>
#include <linker/shared_object.h>
#include <linker/symbol.h>
#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/mp/thread.h>
#include <sys/types.h>



extern const elf_dyn_t _DYNAMIC[];



u64 main(const u64* data){
	const char* path=(data[0]?(const char*)(data[1]):"");
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
	if (!phdr_entries||!phdr_entry_size||!phdr_entry_count){
		sys_io_print("No PHDR supplied to the dynamic linker\n");
		sys_thread_stop(0);
	}
	if (!entry_address){
		sys_io_print("No entry address supplied to the dynamic linker\n");
		sys_thread_stop(0);
	}
	const elf_dyn_t* dynamic_section=NULL;
	const char* interpreter="";
	for (u16 i=0;i<phdr_entry_count;i++){
		const elf_phdr_t* program_header=phdr_entries+i*phdr_entry_size;
		if (program_header->p_type==PT_DYNAMIC){
			dynamic_section=(void*)(program_header->p_vaddr);
		}
		else if (program_header->p_type==PT_INTERP){
			interpreter=(void*)(program_header->p_vaddr);
		}
	}
	shared_object_init(interpreter_image_base,_DYNAMIC,interpreter,0);
	shared_object_t* so=shared_object_load("libsys.so",SHARED_OBJECT_FLAG_RESOLVE_GOT);
	if (so){
		alloc_change_backend((void*)symbol_lookup_by_name_in_shared_object(so,"sys_heap_realloc"));
	}
	shared_object_init(0,dynamic_section,path,0);
	return entry_address;
}
