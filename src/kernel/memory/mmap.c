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
	u64 out=0;
	lock_acquire_exclusive(&(mmap->lock));
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
		goto _cleanup;
	}
	if (address){
		if (region->is_used||address-region->offset+length>region->length){
			goto _cleanup;
		}
	}
	else{
		address=region->offset;
	}
	if (address==region->offset){
		_insert_region_after_anchor(region,0,address+length,region->length-length);
		region->is_used=1;
		region->length=length;
	}
	else{
		_insert_region_after_anchor(region,0,address+length,region->offset+region->length-length-address);
		_insert_region_after_anchor(region,1,address,length);
		region->length=address-region->offset;
	}
	out=address;
_cleanup:
	lock_release_exclusive(&(mmap->lock));
	return out;
}
