#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "mmap"



static lock_t _mmap_lock=LOCK_INIT_STRUCT;
static u64 _mmap_start_address;
static u64 _mmap_end_address;



void mmap_init(void){
	LOG("Initializing user mmap...");
	_mmap_start_address=0;
	_mmap_end_address=0;
}



void mmap_set_range(u64 from,u64 to){
	LOG("Resetting user mmap range to %p - %p...",from,to);
	lock_acquire(&_mmap_lock);
	_mmap_start_address=pmm_align_up_address(from);
	_mmap_end_address=pmm_align_up_address(to);
	lock_release(&_mmap_lock);
}



u64 mmap_alloc(u64 length){
	length=pmm_align_up_address(length);
	lock_acquire(&_mmap_lock);
	if (_mmap_start_address+length>_mmap_end_address){
		lock_release(&_mmap_lock);
		return 0;
	}
	u64 out=_mmap_start_address;
	for (u64 i=0;i<length;i+=PAGE_SIZE){
		vmm_map_page(&vmm_user_pagemap,pmm_alloc(1,PMM_COUNTER_USER),out+i,VMM_PAGE_FLAG_NOEXECUTE|(1ull<<VMM_PAGE_COUNT_SHIFT)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	_mmap_start_address+=length;
	lock_release(&_mmap_lock);
	return out;
}



_Bool mmap_dealloc(u64 address,u64 length){
	u64 offset=address&(PAGE_SIZE-1);
	address-=offset;
	length=pmm_align_up_address(length+offset);
	_Bool out=1;
	for (u64 i=0;i<length;i+=PAGE_SIZE){
		out&=vmm_unmap_page(&vmm_user_pagemap,address+i);
	}
	return out;
}
