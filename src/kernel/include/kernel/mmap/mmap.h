#ifndef _KERNEL_MMAP_MMAP_H_
#define _KERNEL_MMAP_MMAP_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/memory/vmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define MMAP_REGION_FLAG_FORCE 1
#define MMAP_REGION_FLAG_VMM_USER 2
#define MMAP_REGION_FLAG_VMM_WRITE 4
#define MMAP_REGION_FLAG_VMM_EXEC 8
#define MMAP_REGION_FLAG_COMMIT 16
#define MMAP_REGION_FLAG_EXTERNAL 32
#define MMAP_REGION_FLAG_NO_WRITEBACK 64
#define MMAP_REGION_FLAG_STACK 128
#define MMAP_REGION_FLAG_NO_ALLOC 256



typedef struct _MMAP_FREE_REGION{
	rb_tree_node_t rb_node;
	struct _MMAP_FREE_REGION* prev;
	struct _MMAP_FREE_REGION* next;
	struct _MMAP_LENGTH_GROUP* group;
} mmap_free_region_t;



typedef struct _MMAP_LENGTH_GROUP{
	rb_tree_node_t rb_node;
	mmap_free_region_t* head;
} mmap_length_group_t;



typedef struct _MMAP_REGION{
	rb_tree_node_t rb_node;
	u64 length;
	u32 flags;
	vfs_node_t* file;
} mmap_region_t;



typedef struct _MMAP{
	rwlock_t lock;
	vmm_pagemap_t* pagemap;
	u64 bottom_address;
	u64 break_address;
	u64 heap_address;
	u64 top_address;
	rb_tree_t address_tree;
	rb_tree_t length_tree;
	rb_tree_t free_address_tree;
} mmap_t;



mmap_t* mmap_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address);



void mmap_deinit(mmap_t* mmap);



mmap_region_t* mmap_alloc(mmap_t* mmap,u64 address,u64 length,u32 flags,vfs_node_t* file);



bool mmap_dealloc(mmap_t* mmap,u64 address,u64 length);



void mmap_dealloc_region(mmap_t* mmap,mmap_region_t* region);



mmap_region_t* mmap_lookup(mmap_t* mmap,u64 address);



mmap_region_t* mmap_map_to_kernel(mmap_t* mmap,u64 address,u64 length);



u64 mmap_handle_pf(mmap_t* mmap,u64 address,void* isr_state);



void mmap_unmap_address(mmap_t* mmap,u64 address);



#endif
