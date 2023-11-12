#ifndef _CORE_MEMORY_H_
#define _CORE_MEMORY_H_ 1
#include <core/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



#endif
