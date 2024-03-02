#ifndef _KERNEL_MMAP_MMAP_H_
#define _KERNEL_MMAP_MMAP_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/vmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define MMAP2_REGION_FLAG_FORCE 1
#define MMAP2_REGION_FLAG_VMM_USER 2
#define MMAP2_REGION_FLAG_VMM_WRITE 4
#define MMAP2_REGION_FLAG_VMM_EXEC 8
#define MMAP2_REGION_FLAG_COMMIT 16



typedef struct _MMAP2_REGION{
	rb_tree_node_t rb_node;
	u64 length;
} mmap2_region_t;



typedef struct _MMAP2{
	spinlock_t lock;
	vmm_pagemap_t* pagemap;
	u64 bottom_address;
	u64 break_address;
	u64 heap_address;
	u64 top_address;
	u64 stack_top_address;
	u64 stack_size;
	rb_tree_t address_tree;
} mmap2_t;



mmap2_t* mmap2_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address,u64 stack_top_address,u64 stack_size);



void mmap2_deinit(mmap2_t* mmap);



mmap2_region_t* mmap2_alloc(mmap2_t* mmap,u64 address,u64 length,u32 flags);



_Bool mmap2_dealloc(mmap2_t* mmap,u64 address,u64 length);



void mmap2_dealloc_region(mmap2_t* mmap,mmap2_region_t* region);



_Bool mmap2_handle_pf(mmap2_t* mmap,u64 address);



#endif
