#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



#endif
