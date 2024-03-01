#ifndef _KERNEL_MMAP_MMAP_H_
#define _KERNEL_MMAP_MMAP_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



typedef struct _MMAP2{
	spinlock_t lock;
	vmm_pagemap_t* pagemap;
	u64 bottom_address;
	u64 break_address;
	u64 heap_address;
	u64 top_address;
	u64 stack_top_address;
	u64 stack_size;
} mmap2_t;



mmap2_t* mmap2_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address,u64 stack_top_address,u64 stack_size);



void mmap2_deinit(mmap2_t* mmap);



#endif
