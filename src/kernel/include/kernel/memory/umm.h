#ifndef _KERNEL_MEMORY_UMM_H_
#define _KERNEL_MEMORY_UMM_H_ 1
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



void umm_init(void);



void* umm_alloc(u32 size);



#endif
