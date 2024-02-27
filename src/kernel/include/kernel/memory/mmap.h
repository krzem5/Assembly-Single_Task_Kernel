#ifndef _KERNEL_MEMORY_MMAP_H_
#define _KERNEL_MEMORY_MMAP_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define MMAP_REGION_FLAG_USED 1
#define MMAP_REGION_FLAG_VMM_READWRITE 2
#define MMAP_REGION_FLAG_VMM_USER 4
#define MMAP_REGION_FLAG_VMM_NOEXECUTE 8
#define MMAP_REGION_FLAG_COMMIT 16
#define MMAP_REGION_FLAG_NO_FILE_WRITEBACK 32
#define MMAP_REGION_FLAG_EXTERNAL 64

#define MMAP_REGION_FILE_OFFSET_SHIFT 6
#define MMAP_REGION_FILE_OFFSET(offset) (((u64)(offset))<<MMAP_REGION_FILE_OFFSET_SHIFT)



typedef struct _MMAP_REGION{
	rb_tree_node_t rb_node;
	u64 flags;
	u64 length;
	struct _MMAP_REGION* prev;
	struct _MMAP_REGION* next;
	struct _MMAP_LENGTH_GROUP* group;
	struct _MMAP_REGION* group_prev;
	struct _MMAP_REGION* group_next;
	pmm_counter_descriptor_t* pmm_counter;
	vfs_node_t* file;
} mmap_region_t;



typedef struct _MMAP_LENGTH_GROUP{
	rb_tree_node_t rb_node;
	mmap_region_t* head;
} mmap_length_group_t;



typedef struct _MMAP{
	vmm_pagemap_t* pagemap;
	spinlock_t lock;
	rb_tree_t offset_tree;
	rb_tree_t length_tree;
} mmap_t;



void mmap_init(vmm_pagemap_t* pagemap,u64 low,u64 high,mmap_t* out);



void mmap_deinit(mmap_t* mmap);



mmap_region_t* mmap_alloc(mmap_t* mmap,u64 address,u64 length,pmm_counter_descriptor_t* pmm_counter,u64 flags,vfs_node_t* file,u64 physical_address);



_Bool mmap_dealloc(mmap_t* mmap,u64 address,u64 length);



_Bool mmap_dealloc_region(mmap_t* mmap,mmap_region_t* region);



_Bool mmap_change_flags(mmap_t* mmap,u64 address,u64 length,u64 vmm_set_flags,u64 vmm_clear_flags);



_Bool mmap_set_memory(mmap_t* mmap,mmap_region_t* region,u64 offset,const void* data,u64 length);



mmap_region_t* mmap_map_region(mmap_t* mmap,mmap_region_t* region,u64 offset,u64 length);



mmap_region_t* mmap_lookup(mmap_t* mmap,u64 address);



u64 mmap_get_vmm_flags(mmap_region_t* region);



#endif
