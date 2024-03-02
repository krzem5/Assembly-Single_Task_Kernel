#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



static omm_allocator_t* _mmap_allocator=NULL;
static pmm_counter_descriptor_t* _mmap_stack_pmm_counter=NULL;



mmap2_t* mmap2_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address,u64 stack_top_address,u64 stack_size){
	if (!_mmap_allocator){
		_mmap_allocator=omm_init("mmap",sizeof(mmap2_t),8,4,pmm_alloc_counter("omm_mmap"));
		spinlock_init(&(_mmap_allocator->lock));
	}
	if (!_mmap_stack_pmm_counter){
		_mmap_stack_pmm_counter=pmm_alloc_counter("mmap_stack");
	}
	mmap2_t* out=omm_alloc(_mmap_allocator);
	spinlock_init(&(out->lock));
	out->pagemap=pagemap;
	out->bottom_address=bottom_address;
	out->break_address=bottom_address;
	out->heap_address=top_address;
	out->top_address=top_address;
	out->stack_top_address=stack_top_address;
	out->stack_size=stack_size;
	rb_tree_init(&(out->address_tree));
	return out;
}



void mmap2_deinit(mmap2_t* mmap){
	omm_dealloc(_mmap_allocator,mmap);
}



mmap2_region_t* mmap2_alloc(mmap2_t* mmap,u64 address,u64 length,u32 flags){
	panic("mmap2_alloc");
}



_Bool mmap2_dealloc(mmap2_t* mmap,u64 address,u64 length){
	panic("mmap2_dealloc");
}



void mmap2_dealloc_region(mmap2_t* mmap,mmap2_region_t* region){
	panic("mmap2_dealloc_region");
}



_Bool mmap2_handle_pf(mmap2_t* mmap,u64 address){
	if (!mmap){
		return 0;
	}
	spinlock_acquire_exclusive(&(mmap->lock));
	if (address<mmap->stack_top_address&&address>=mmap->stack_top_address-mmap->stack_size){
		WARN("Stack: %p",address);
		vmm_map_page(mmap->pagemap,pmm_alloc(1,_mmap_stack_pmm_counter,0),address,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
		spinlock_release_exclusive(&(mmap->lock));
		return 0;
	}
	WARN("%p",address);
	spinlock_release_exclusive(&(mmap->lock));
	return 0;
}
