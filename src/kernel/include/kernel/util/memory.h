#ifndef _KERNEL_UTIL_MEMORY_H_
#define _KERNEL_UTIL_MEMORY_H_ 1
#include <kernel/types.h>



void mem_copy(void* dst,const void* src,u64 length);



void mem_fill(void* ptr,u64 length,u8 value);



#endif
