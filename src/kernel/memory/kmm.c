#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "kmm"



static lock_t _kmm_lock=LOCK_INIT_STRUCT;
static u64 KERNEL_CORE_DATA _kmm_top;
static u64 KERNEL_CORE_DATA _kmm_max_top;
static _Bool KERNEL_CORE_DATA _kmm_buffer_not_ended=0;



static void KERNEL_CORE_CODE _resize_stack(void){
	while (_kmm_top>_kmm_max_top){
		vmm_map_page(&vmm_kernel_pagemap,pmm_alloc(1,PMM_COUNTER_KMM),_kmm_max_top,VMM_PAGE_FLAG_PRESENT|VMM_PAGE_FLAG_READWRITE);
		_kmm_max_top+=PAGE_SIZE;
	}
}



void KERNEL_CORE_CODE kmm_init(void){
	LOG_CORE("Initializing kernel memory manager...");
	_kmm_top=pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset());
	_kmm_max_top=pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset());
}



void* KERNEL_CORE_CODE kmm_allocate(u32 size){
	lock_acquire_exclusive(&_kmm_lock);
	if (_kmm_buffer_not_ended){
		ERROR_CORE("Buffer in use");
		for (;;);
	}
	void* out=(void*)_kmm_top;
	_kmm_top+=(size+7)&0xfffffffffffffff8ull;
	_resize_stack();
	lock_release_exclusive(&_kmm_lock);
	return out;
}



void* KERNEL_CORE_CODE kmm_allocate_buffer(void){
	lock_acquire_exclusive(&_kmm_lock);
	if (_kmm_buffer_not_ended){
		ERROR_CORE("Buffer already in use");
		for (;;);
	}
	_kmm_buffer_not_ended=1;
	lock_release_exclusive(&_kmm_lock);
	return (void*)_kmm_top;
}



void KERNEL_CORE_CODE kmm_grow_buffer(u32 size){
	lock_acquire_exclusive(&_kmm_lock);
	if (!_kmm_buffer_not_ended){
		ERROR_CORE("Buffer not in use");
		for (;;);
	}
	_kmm_top+=size;
	_resize_stack();
	lock_release_exclusive(&_kmm_lock);
}



void KERNEL_CORE_CODE kmm_shrink_buffer(u32 size){
	lock_acquire_exclusive(&_kmm_lock);
	if (!_kmm_buffer_not_ended){
		ERROR_CORE("Buffer not in use");
		for (;;);
	}
	_kmm_top-=size;
	lock_release_exclusive(&_kmm_lock);
}



void KERNEL_CORE_CODE kmm_end_buffer(void){
	lock_acquire_exclusive(&_kmm_lock);
	if (!_kmm_buffer_not_ended){
		ERROR_CORE("KMM buffer not in use");
		for (;;);
	}
	_kmm_buffer_not_ended=0;
	_kmm_top=(_kmm_top+7)&0xfffffffffffffff8ull;
	_resize_stack();
	lock_release_exclusive(&_kmm_lock);
}
