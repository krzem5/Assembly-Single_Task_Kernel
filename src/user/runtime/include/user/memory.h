#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2

#define MEMORY_RANGE_TYPE_NORMAL 1
#define MEMORY_RANGE_TYPE_UNUSABLE 2
#define MEMORY_RANGE_TYPE_ACPI_TABLES 3
#define MEMORY_RANGE_TYPE_ACPI_NVS 4
#define MEMORY_RANGE_TYPE_BAD_MEMORY 5



typedef struct _MEMORY_COUNTER{
	char name[16];
	u64 count;
} memory_counter_t;



typedef struct _MEMORY_OBJECT_COUNTER{
	char name[16];
	u64 allocation_count;
	u64 deallocation_count;
} memory_object_counter_t;



typedef struct _MEMORY_RANGE{
	u64 base_address;
	u64 length;
	u32 type;
} memory_range_t;



_Bool memory_get_range(u32 index,memory_range_t* out);



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



u32 memory_get_counter_count(void);



_Bool memory_get_counter(u32 counter,memory_counter_t* out);



u32 memory_get_object_counter_count(void);



_Bool memory_get_object_counter(u32 counter,memory_object_counter_t* out);



#endif
