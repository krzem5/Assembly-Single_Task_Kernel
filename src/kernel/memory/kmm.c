#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "kmm"



static pmm_counter_descriptor_t _kmm_pmm_counter=PMM_COUNTER_INIT_STRUCT("kmm");



static lock_t _kmm_lock=LOCK_INIT_STRUCT;
static u64 _kmm_top;
static u64 _kmm_max_top;
static _Bool _kmm_buffer_not_ended=0;
static _Bool _kmm_frozen=0;



static void _resize_stack(void){
	while (_kmm_top>_kmm_max_top){
		u64 page=pmm_alloc(1,&_kmm_pmm_counter,0);
		vmm_map_page(&vmm_kernel_pagemap,page,_kmm_max_top,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
		_kmm_max_top+=PAGE_SIZE;
	}
}



void kmm_init(void){
	LOG("Initializing kernel memory manager...");
	_kmm_top=kernel_data.first_free_address+kernel_get_offset();
	_kmm_max_top=kernel_data.first_free_address+kernel_get_offset();
}



void kmm_freeze_allocator(void){
	if (_kmm_frozen){
		panic("kmm: allocator is frozen");
	}
	LOG("Freezing kernel memory allocator...");
	_kmm_frozen=1;
	INFO("Total KMM size: %v",_kmm_max_top-kernel_get_offset()-kernel_data.first_free_address);
	kernel_data.first_free_address=_kmm_max_top-kernel_get_offset();
}



void* kmm_alloc(u32 size){
	lock_acquire_exclusive(&_kmm_lock);
	if (_kmm_frozen){
		panic("kmm: allocator is frozen");
	}
	if (_kmm_buffer_not_ended){
		panic("kmm: buffer in use");
	}
	void* out=(void*)_kmm_top;
	_kmm_top+=(size+7)&0xfffffffffffffff8ull;
	_resize_stack();
	lock_release_exclusive(&_kmm_lock);
	return out;
}



void* kmm_alloc_buffer(void){
	lock_acquire_exclusive(&_kmm_lock);
	if (_kmm_frozen){
		panic("kmm: allocator is frozen");
	}
	if (_kmm_buffer_not_ended){
		panic("kmm: buffer already in use");
	}
	_kmm_buffer_not_ended=1;
	lock_release_exclusive(&_kmm_lock);
	return (void*)_kmm_top;
}



void kmm_grow_buffer(u32 size){
	lock_acquire_exclusive(&_kmm_lock);
	if (_kmm_frozen){
		panic("kmm: allocator is frozen");
	}
	if (!_kmm_buffer_not_ended){
		panic("kmm: buffer not in use");
	}
	_kmm_top+=size;
	_resize_stack();
	lock_release_exclusive(&_kmm_lock);
}



void kmm_end_buffer(void){
	lock_acquire_exclusive(&_kmm_lock);
	if (_kmm_frozen){
		panic("kmm: allocator is frozen");
	}
	if (!_kmm_buffer_not_ended){
		panic("kmm: buffer not in use");
	}
	_kmm_buffer_not_ended=0;
	_kmm_top=(_kmm_top+7)&0xfffffffffffffff8ull;
	_resize_stack();
	lock_release_exclusive(&_kmm_lock);
}
