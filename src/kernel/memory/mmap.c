#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/mmap.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



#define LOWEST_ADDRESS 0x0000000000001000ull
#define HIGHEST_ADDRESS 0x0000800000000000ull



static vmm_memory_map_region_t* _insert_region_after_anchor(vmm_memory_map_region_t* anchor,_Bool is_used,u64 offset,u64 length){
	vmm_memory_map_region_t* out=kmm_alloc(sizeof(vmm_memory_map_region_t));
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



void vmm_memory_map_init(vmm_memory_map_t* out){
	lock_init(&(out->lock));
	out->first=_insert_region_after_anchor(NULL,0,LOWEST_ADDRESS,HIGHEST_ADDRESS-LOWEST_ADDRESS);
}



void vmm_memory_map_deinit(vmm_memory_map_t* mmap);



u64 vmm_memory_map_reserve(vmm_memory_map_t* mmap,u64 address,u64 length){
	if ((address|length)&(PAGE_SIZE-1)){
		panic("vmm_memory_map_reserve: unaligned arguments",1);
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
		panic("vmm_memory_map_release: unaligned arguments",1);
		return 0;
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
		panic("vmm_memory_map_release: partial release is unimplemented",0);
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
