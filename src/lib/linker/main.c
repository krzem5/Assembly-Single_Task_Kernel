#include <core/elf.h>
#include <core/io.h>
#include <core/types.h>
#include <linker/shared_object.h>



static const elf_dyn_t* _load_root_shared_object_data(const u64* data,u64* out){
	const void* phdr_entries=NULL;
	u64 phdr_entry_size=0;
	u64 phdr_entry_count=0;
	*out=0;
	for (data+=(data[0]+1);data[0];data++);
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
		else if (data[0]==AT_ENTRY){
			*out=data[1];
		}
	}
	if (!phdr_entries||!phdr_entry_size||!phdr_entry_count){
		printf("No PHDR supplied to the dynamic linker\n");
	}
	if (!(*out)){
		printf("No entry address supplied to the dynamic linker\n");
	}
	for (u16 i=0;i<phdr_entry_count;i++){
		const elf_phdr_t* program_header=phdr_entries+i*phdr_entry_size;
		if (program_header->p_type==PT_DYNAMIC){
			return (void*)(program_header->p_vaddr);
		}
	}
	return NULL;
}



u64 main(const u64* data){
	u64 entry_address;
	shared_object_init(0,_load_root_shared_object_data(data,&entry_address));
	return entry_address;
}
