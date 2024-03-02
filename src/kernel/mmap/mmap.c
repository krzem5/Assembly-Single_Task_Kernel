#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



static pmm_counter_descriptor_t* _mmap_pmm_counter=NULL;
static omm_allocator_t* _mmap_allocator=NULL;
static omm_allocator_t* _mmap_region_allocator=NULL;



static void _dealloc_region(mmap2_t* mmap,mmap2_region_t* region){
	//
}



mmap2_t* mmap2_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address){
	if (!_mmap_pmm_counter){
		_mmap_pmm_counter=pmm_alloc_counter("mmap");
	}
	if (!_mmap_allocator){
		_mmap_allocator=omm_init("mmap",sizeof(mmap2_t),8,4,pmm_alloc_counter("omm_mmap"));
		spinlock_init(&(_mmap_allocator->lock));
	}
	if (!_mmap_region_allocator){
		_mmap_region_allocator=omm_init("mmap_region",sizeof(mmap2_region_t),8,4,pmm_alloc_counter("omm_mmap_region"));
		spinlock_init(&(_mmap_region_allocator->lock));
	}
	mmap2_t* out=omm_alloc(_mmap_allocator);
	spinlock_init(&(out->lock));
	out->pagemap=pagemap;
	out->bottom_address=bottom_address;
	out->break_address=bottom_address;
	out->heap_address=top_address;
	out->top_address=top_address;
	rb_tree_init(&(out->address_tree));
	return out;
}



void mmap2_deinit(mmap2_t* mmap){
	omm_dealloc(_mmap_allocator,mmap);
}



KERNEL_PUBLIC mmap2_region_t* mmap2_alloc(mmap2_t* mmap,u64 address,u64 length,u32 flags){
	if ((address|length)&(PAGE_SIZE-1)){
		return NULL;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	if (!address){
		mmap->heap_address-=length;
		address=mmap->heap_address;
	}
	else if (!(flags&MMAP2_REGION_FLAG_FORCE)){
		panic("mmap2_alloc: check user address and length");
	}
	mmap2_region_t* out=omm_alloc(_mmap_region_allocator);
	out->rb_node.key=address;
	out->length=length;
	out->flags=flags;
	rb_tree_insert_node(&(mmap->address_tree),&(out->rb_node));
	spinlock_release_exclusive(&(mmap->lock));
	if (flags&MMAP2_REGION_FLAG_COMMIT){
		for (u64 offset=address;offset<address+length;offset+=PAGE_SIZE){
			mmap2_handle_pf(mmap,offset);
		}
	}
	return out;
}



KERNEL_PUBLIC _Bool mmap2_dealloc(mmap2_t* mmap,u64 address,u64 length){
	panic("mmap2_dealloc");
}



KERNEL_PUBLIC void mmap2_dealloc_region(mmap2_t* mmap,mmap2_region_t* region){
	spinlock_acquire_exclusive(&(mmap->lock));
	rb_tree_remove_node(&(mmap->address_tree),&(region->rb_node));
	_dealloc_region(mmap,region);
	WARN("Push %p, %v",region->rb_node.key,region->length);
	omm_dealloc(_mmap_region_allocator,region);
	spinlock_release_exclusive(&(mmap->lock));
}



KERNEL_PUBLIC mmap2_region_t* mmap2_lookup(mmap2_t* mmap,u64 address){
	if (!mmap){
		return NULL;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap2_region_t* out=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (out&&out->rb_node.key+out->length<=address){
		out=NULL;
	}
	spinlock_release_exclusive(&(mmap->lock));
	return out;
}



KERNEL_PUBLIC mmap2_region_t* mmap2_map_to_kernel(mmap2_t* mmap,u64 address,u64 length){
	mmap2_region_t* out=mmap2_alloc(process_kernel->mmap2,0,length,MMAP2_REGION_FLAG_EXTERNAL|MMAP2_REGION_FLAG_VMM_WRITE);
	for (u64 offset=address;offset<address+length;offset+=PAGE_SIZE){
		u64 physical_address=vmm_virtual_to_physical(mmap->pagemap,offset);
		if (!physical_address){
			physical_address=mmap2_handle_pf(mmap,offset);
			if (!physical_address){
				panic("mmap2_map_to_kernel: invalid address");
			}
		}
		vmm_map_page(&(process_kernel->pagemap),physical_address,out->rb_node.key+offset-address,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	return out;
}



u64 mmap2_handle_pf(mmap2_t* mmap,u64 address){
	if (!mmap){
		return 0;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	mmap2_region_t* region=(void*)rb_tree_lookup_decreasing_node(&(mmap->address_tree),address);
	if (region&&region->rb_node.key+region->length>address){
		u64 out=pmm_alloc(1,_mmap_pmm_counter,0);
		u64 flags=VMM_PAGE_FLAG_PRESENT;
		if (region->flags&MMAP2_REGION_FLAG_VMM_USER){
			flags|=VMM_PAGE_FLAG_USER;
		}
		if (region->flags&MMAP2_REGION_FLAG_VMM_WRITE){
			flags|=VMM_PAGE_FLAG_READWRITE;
		}
		if (!(region->flags&MMAP2_REGION_FLAG_VMM_EXEC)){
			flags|=VMM_PAGE_FLAG_NOEXECUTE;
		}
		WARN("%p: %p, %p",address,out,flags);
		vmm_map_page(mmap->pagemap,out,address,flags);
		spinlock_release_exclusive(&(mmap->lock));
		return out;
	}
	spinlock_release_exclusive(&(mmap->lock));
	return 0;
}
