#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2



typedef struct _MEMORY_RANGE{
	u64 base_address;
	u64 length;
} memory_range_t;



_Bool memory_get_range(u32 index,memory_range_t* out);



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



#endif
