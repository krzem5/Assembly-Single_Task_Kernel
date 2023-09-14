#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/mmap.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "mmap"



static vmm_memory_map_region_t* _alloc_region(u64 offset,u64 length){
	vmm_memory_map_region_t* out=kmm_alloc(sizeof(vmm_memory_map_region_t));
	out->is_used=0;
	out->offset=offset;
	out->length=length;
	return out;
}



void vmm_memory_map_init(u64 offset,u64 length,vmm_memory_map_t* out){
	lock_init(&(out->lock));
	vmm_memory_map_region_t* region=_alloc_region(offset,length);
	region->next=NULL;
	region->prev=NULL;
	region->next_buddy=NULL;
	region->prev_buddy=NULL;
	out->first=region;
	out->empty_head=region;
	out->used_head=NULL;
}



void vmm_memory_map_deinit(vmm_memory_map_t* mmap);



u64 vmm_memory_map_reserve(vmm_memory_map_t* mmap,u64 address,u64 length){
	extern void panic(const char*,_Bool);
	if (!address){
		panic("vmm_memory_map_reserve: find_free",0);
	}
	else{
		panic("vmm_memory_map_reserve: check if address is free",0);
	}
	return 0;
}
