#ifndef _LINKER_SHARED_OBJECT_H_
#define _LINKER_SHARED_OBJECT_H_ 1
#include <sys/elf/elf.h>
#include <sys/types.h>



#define LINKER_SHARED_OBJECT_FLAG_RESOLVE_GOT 1



typedef struct _LINKER_SHARED_OBJECT_DYNAMIC_SECTION_DATA{
	bool has_needed_libraries;
	u64 plt_relocation_size;
	u64* plt_got;
	const elf_hash_t* hash_table;
	const char* string_table;
	const void* symbol_table;
	const void* relocations;
	u64 relocation_size;
	u64 relocation_entry_size;
	u64 symbol_table_entry_size;
	void* init;
	void* fini;
	u64 plt_relocation_entry_size;
	const void* plt_relocations;
	const void* init_array;
	u64 init_array_size;
	const void* fini_array;
	u64 fini_array_size;
	const elf_gnu_hash_t* gnu_hash_table;
	const u64* gnu_hash_table_bloom_filter;
	const u32* gnu_hash_table_buckets;
	const u32* gnu_hash_table_values;
} linker_shared_object_dynamic_section_data_t;



typedef struct _LINKER_SHARED_OBJECT{
	struct _LINKER_SHARED_OBJECT* prev;
	struct _LINKER_SHARED_OBJECT* next;
	char path[256];
	u64 image_base;
	linker_shared_object_dynamic_section_data_t dynamic_section;
#ifdef KERNEL_COVERAGE
	u64 gcov_info_base;
	u64 gcov_info_size;
#endif
} linker_shared_object_t;



extern linker_shared_object_t* linker_shared_object_root;
extern linker_shared_object_t* linker_shared_object_executable;



linker_shared_object_t* linker_shared_object_init(u64 image_base,const elf_dyn_t* dynamic_section,const char* path,u32 flags);



linker_shared_object_t* linker_shared_object_load(const char* name,u32 flags);



void linker_shared_object_execute_fini(void);



#endif
