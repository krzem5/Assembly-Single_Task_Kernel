#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2



typedef struct _MEMORY_COUNTER_DATA{
	char name[64];
	u64 count;
} memory_counter_data_t;



typedef struct _MEMORY_OBJECT_ALLOCATOR_DATA{
	char name[64];
	u64 allocation_count;
	u64 deallocation_count;
} memory_object_allocator_data_t;



typedef struct _MEMORY_RANGE{
	u64 base_address;
	u64 length;
} memory_range_t;



_Bool memory_get_range(u32 index,memory_range_t* out);



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



_Bool memory_counter_get_data(u64 handle,memory_counter_data_t* out);



_Bool memory_object_allocator_get_data(u64 handle,memory_object_allocator_data_t* out);



#endif
