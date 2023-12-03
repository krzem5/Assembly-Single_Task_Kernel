#ifndef _KERNEL_MEMORY_AMM_H_
#define _KERNEL_MEMORY_AMM_H_ 1
#include <kernel/types.h>



void* amm_alloc(u32 length);



void amm_dealloc(void* ptr);



void* amm_realloc(void* ptr,u32 length);



#endif
