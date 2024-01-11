#include <linker/shared_object.h>
#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/types.h>



static const elf_dyn_t* _find_dynamic_section_and_entry_address(const u64* data,u64* entry_address){
	const void* phdr_entries=NULL;
	u64 phdr_entry_size=0;
	u64 phdr_entry_count=0;
	*entry_address=0;
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
			*entry_address=data[1];
		}
	}
	if (!phdr_entries||!phdr_entry_size||!phdr_entry_count){
		sys_io_print("No PHDR supplied to the dynamic linker\n");
	}
	if (!(*entry_address)){
		sys_io_print("No entry address supplied to the dynamic linker\n");
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
	shared_object_init(0,_find_dynamic_section_and_entry_address(data,&entry_address),"");
	return entry_address;
}
