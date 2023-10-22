#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



PMM_DECLARE_COUNTER2(OMM_MMAP_RANGE);



static omm_allocator_t _mmap_region_allocator=OMM_ALLOCATOR_INIT_STRUCT("mmap_range",sizeof(vmm_memory_map_region_t),8,4,&_pmm_counter_descriptor_OMM_MMAP_RANGE);



static vmm_memory_map_region_t* _insert_region_after_anchor(vmm_memory_map_region_t* anchor,_Bool is_used,u64 offset,u64 length){
	vmm_memory_map_region_t* out=omm_alloc(&_mmap_region_allocator);
	out->is_used=is_used;
	out->offset=offset;
	out->length=length;
	if (anchor){
		out->prev=anchor;
		out->next=anchor->next;
		if (anchor->next){
			anchor->next->prev=out;
		}
		anchor->next=out;
	}
	else{
		out->prev=NULL;
		out->next=NULL;
	}
	return out;
}



static void _delete_next_region(vmm_memory_map_region_t* region){
	region=region->next;
	region->prev->next=region->next;
	if (region->next){
		region->next->prev=region->prev;
	}
}



void vmm_memory_map_init(u64 low,u64 high,vmm_memory_map_t* out){
	lock_init(&(out->lock));
	out->first=_insert_region_after_anchor(NULL,0,low,high-low);
}



void vmm_memory_map_deinit(vmm_pagemap_t* pagemap,vmm_memory_map_t* mmap){
	lock_acquire_exclusive(&(mmap->lock));
	for (vmm_memory_map_region_t* region=mmap->first;region;){
		if (region->is_used){
			vmm_release_pages(pagemap,region->offset,region->length>>PAGE_SIZE_SHIFT);
		}
		vmm_memory_map_region_t* next=region->next;
		omm_dealloc(&_mmap_region_allocator,region);
		region=next;
	}
	mmap->first=NULL;
	lock_release_exclusive(&(mmap->lock));
}



u64 vmm_memory_map_reserve(vmm_memory_map_t* mmap,u64 address,u64 length){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("vmm_memory_map_reserve: unaligned arguments");
	}
	if (!length){
		return 0;
	}
	lock_acquire_shared(&(mmap->lock));
	vmm_memory_map_region_t* region=mmap->first;
	if (address){
		while (region&&region->offset+region->length<=address){
			region=region->next;
		}
	}
	else{
		while (region&&(region->is_used||region->length<length)){
			region=region->next;
		}
	}
	if (!region){
		lock_release_shared(&(mmap->lock));
		return 0;
	}
	if (!address){
		address=region->offset;
	}
	else if (region->is_used||address-region->offset+length>region->length){
		lock_release_shared(&(mmap->lock));
		return 0;
	}
	lock_shared_to_exclusive(&(mmap->lock));
	_insert_region_after_anchor(region,0,address+length,region->offset+region->length-length-address);
	if (address==region->offset){
		region->is_used=1;
		region->length=length;
	}
	else{
		_insert_region_after_anchor(region,1,address,length);
		region->length=address-region->offset;
	}
	lock_release_exclusive(&(mmap->lock));
	return address;
}



_Bool vmm_memory_map_release(vmm_memory_map_t* mmap,u64 address,u64 length){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("vmm_memory_map_release: unaligned arguments");
	}
	lock_acquire_shared(&(mmap->lock));
	vmm_memory_map_region_t* region=mmap->first;
	while (region&&region->offset+region->length<=address){
		region=region->next;
	}
	if (!region){
		lock_release_shared(&(mmap->lock));
		return 0;
	}
	if (region->offset!=address||region->length!=length){
		panic("vmm_memory_map_release: partial release is unimplemented");
	}
	lock_shared_to_exclusive(&(mmap->lock));
	region->is_used=0;
	if (region->prev&&!region->prev->is_used){
		region=region->prev;
		_delete_next_region(region);
	}
	if (region->next&&!region->next->is_used){
		_delete_next_region(region);
	}
	lock_release_exclusive(&(mmap->lock));
	return 1;
}
