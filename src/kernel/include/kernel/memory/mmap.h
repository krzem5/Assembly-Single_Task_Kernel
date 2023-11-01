#ifndef _KERNEL_MEMORY_MMAP_H_
#define _KERNEL_MEMORY_MMAP_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/vmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



typedef struct _VMM_MEMORY_MAP_REGION{
	rb_tree_node_t rb_node;
	_Bool is_used;
	u64 length;
	struct _VMM_MEMORY_MAP_REGION* prev;
	struct _VMM_MEMORY_MAP_REGION* next;
	struct _VMM_MEMORY_MAP_LENGTH_GROUP* group;
	struct _VMM_MEMORY_MAP_REGION* group_prev;
	struct _VMM_MEMORY_MAP_REGION* group_next;
} vmm_memory_map_region_t;



typedef struct _VMM_MEMORY_MAP_LENGTH_GROUP{
	rb_tree_node_t rb_node;
	vmm_memory_map_region_t* head;
} vmm_memory_map_length_group_t;



typedef struct _VMM_MEMORY_MAP{
	spinlock_t lock;
	rb_tree_t offset_tree;
	rb_tree_t length_tree;
} vmm_memory_map_t;



void vmm_memory_map_init(u64 low,u64 high,vmm_memory_map_t* out);



void vmm_memory_map_deinit(vmm_pagemap_t* pagemap,vmm_memory_map_t* mmap);



u64 vmm_memory_map_reserve(vmm_memory_map_t* mmap,u64 address,u64 length);



_Bool vmm_memory_map_release(vmm_memory_map_t* mmap,u64 address,u64 length);



#endif
