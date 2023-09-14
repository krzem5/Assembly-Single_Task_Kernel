#ifndef _KERNEL_MEMORY_MMAP_H_
#define _KERNEL_MEMORY_MMAP_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



typedef struct _VMM_MEMORY_MAP_REGION{
	_Bool is_used;
	u64 offset;
	u64 length;
	struct _VMM_MEMORY_MAP_REGION* next;
	struct _VMM_MEMORY_MAP_REGION* prev;
	struct _VMM_MEMORY_MAP_REGION* next_buddy;
	struct _VMM_MEMORY_MAP_REGION* prev_buddy;
} vmm_memory_map_region_t;



typedef struct _VMM_MEMORY_MAP{
	lock_t lock;
	vmm_memory_map_region_t* first;
	vmm_memory_map_region_t* empty_head;
	vmm_memory_map_region_t* used_head;
} vmm_memory_map_t;



void vmm_memory_map_init(u64 offset,u64 length,vmm_memory_map_t* out);



void vmm_memory_map_deinit(vmm_memory_map_t* mmap);



u64 vmm_memory_map_reserve(vmm_memory_map_t* mmap,u64 address,u64 length);



#endif
