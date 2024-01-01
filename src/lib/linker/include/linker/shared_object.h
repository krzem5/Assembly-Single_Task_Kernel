#ifndef _LINKER_SHARED_OBJECT_H_
#define _LINKER_SHARED_OBJECT_H_ 1
#include <sys/elf/elf.h>
#include <sys/types.h>



typedef struct _SHARED_OBJECT_DYNAMIC_SECTION_DATA{
	_Bool has_needed_libraries;
	u64 plt_relocation_size;
	u64* plt_got;
	const elf_hash_t* hash_table;
	const char* string_table;
	const void* symbol_table;
	const void* relocations;
	u64 relocation_size;
	u64 relocation_entry_size;
	u64 symbol_table_entry_size;
	u64 plt_relocation_entry_size;
	const void* plt_relocations;
} shared_object_dynamic_section_data_t;



typedef struct _SHARED_OBJECT{
	struct _SHARED_OBJECT* next;
	char path[256];
	u64 image_base;
	shared_object_dynamic_section_data_t dynamic_section;
} shared_object_t;



extern shared_object_t* shared_object_root;



shared_object_t* shared_object_init(u64 image_base,const elf_dyn_t* dynamic_section,const char* path);



shared_object_t* shared_object_load(const char* name);



#endif
