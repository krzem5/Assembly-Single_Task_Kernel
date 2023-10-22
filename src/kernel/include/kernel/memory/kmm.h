#ifndef _KERNEL_MEMORY_KMM_H_
#define _KERNEL_MEMORY_KMM_H_ 1
#include <kernel/types.h>



void kmm_init(void);



void* kmm_alloc(u32 size);



void* kmm_alloc_buffer(void);



void kmm_grow_buffer(u32 size);



void kmm_end_buffer(void);



#endif
