#ifndef _KERNEL_MEMORY_MMAP_H_
#define _KERNEL_MEMORY_MMAP_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



typedef struct _MMAP_REGION{
	rb_tree_node_t rb_node;
	_Bool is_used;
	u64 length;
	struct _MMAP_REGION* prev;
	struct _MMAP_REGION* next;
	struct _MMAP_LENGTH_GROUP* group;
	struct _MMAP_REGION* group_prev;
	struct _MMAP_REGION* group_next;
	pmm_counter_descriptor_t* pmm_counter;
} mmap_region_t;



typedef struct _MMAP_LENGTH_GROUP{
	rb_tree_node_t rb_node;
	mmap_region_t* head;
} mmap_length_group_t;



typedef struct _MMAP{
	spinlock_t lock;
	rb_tree_t offset_tree;
	rb_tree_t length_tree;
} mmap_t;



void mmap_init(u64 low,u64 high,mmap_t* out);



void mmap_deinit(vmm_pagemap_t* pagemap,mmap_t* mmap);



mmap_region_t* mmap_reserve(mmap_t* mmap,u64 address,u64 length,pmm_counter_descriptor_t* pmm_counter);



_Bool mmap_release(mmap_t* mmap,u64 address,u64 length);



#endif
