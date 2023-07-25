#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



void* memory_map(u64 length);



_Bool memory_unmap(void* address,u64 length);



#endif
