#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "mmap"



static omm_allocator_t* _mmap_allocator=NULL;



mmap2_t* mmap2_init(vmm_pagemap_t* pagemap,u64 bottom_address,u64 top_address,u64 stack_top_address,u64 stack_size){
	if (!_mmap_allocator){
		_mmap_allocator=omm_init("mmap",sizeof(mmap2_t),8,4,pmm_alloc_counter("omm_mmap"));
		spinlock_init(&(_mmap_allocator->lock));
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
	return out;
}



void mmap2_deinit(mmap2_t* mmap){
	omm_dealloc(_mmap_allocator,mmap);
}



_Bool mmap2_handle_pf(mmap2_t* mmap,u64 address){
	if (!mmap){
		return 0;
	}
	return 0;
}
