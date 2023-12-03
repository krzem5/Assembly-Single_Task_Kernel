#ifndef _KERNEL_MEMORY_AMM_H_
#define _KERNEL_MEMORY_AMM_H_ 1
#include <kernel/types.h>



typedef struct _AMM_HEADER{
	u64 index;
	u8 data[];
} amm_header_t;



void amm_init(void);



void* amm_alloc(u32 length);



void amm_dealloc(void* ptr);



void* amm_realloc(void* ptr,u32 length);



#endif
