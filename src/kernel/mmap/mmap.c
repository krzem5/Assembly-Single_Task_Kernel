#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



static lock_t _mmap_lock=LOCK_INIT_STRUCT;
static u64 _mmap_start_address;
static u64 _mmap_end_address;



void mmap_set_range(u64 from,u64 to){
	LOG("Resetting user mmap range to %p - %p...",from,to);
	lock_acquire_exclusive(&_mmap_lock);
	_mmap_start_address=pmm_align_up_address(from);
	_mmap_end_address=pmm_align_up_address(to);
	lock_release_exclusive(&_mmap_lock);
}



u64 mmap_alloc(u64 length,u8 flags){
	u64 size=PAGE_SIZE;
	u64 page_flags=VMM_PAGE_FLAG_NOEXECUTE|VMM_MAP_WITH_COUNT|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
	if (flags&MMAP_FLAG_EXTRA_LARGE){
		size=EXTRA_LARGE_PAGE_SIZE;
		length=pmm_align_up_address_extra_large(length);
		_mmap_start_address=pmm_align_up_address_extra_large(_mmap_start_address);
		page_flags|=VMM_PAGE_FLAG_EXTRA_LARGE;
	}
	else if (flags&MMAP_FLAG_LARGE){
		size=LARGE_PAGE_SIZE;
		length=pmm_align_up_address_large(length);
		_mmap_start_address=pmm_align_up_address_large(_mmap_start_address);
		page_flags|=VMM_PAGE_FLAG_LARGE;
	}
	else{
		length=pmm_align_up_address(length);
	}
	lock_acquire_exclusive(&_mmap_lock);
	if (_mmap_start_address+length>_mmap_end_address){
		panic("MMAP: Out of linear memory",1);
		lock_release_exclusive(&_mmap_lock);
		return 0;
	}
	vmm_map_pages(&vmm_user_pagemap,pmm_alloc(length>>PAGE_SIZE_SHIFT,PMM_COUNTER_USER),_mmap_start_address,page_flags,length>>PAGE_SIZE_SHIFT);
	u64 out=_mmap_start_address;
	_mmap_start_address+=length*(size>>PAGE_SIZE_SHIFT);
	lock_release_exclusive(&_mmap_lock);
	return out;
}



_Bool mmap_dealloc(u64 address,u64 length){
	u64 offset=address&(PAGE_SIZE-1);
	address-=offset;
	length=pmm_align_up_address(length+offset);
	_Bool out=1;
	for (u64 i=0;i<length;){
		u64 size=vmm_unmap_page(&vmm_user_pagemap,address+i);
		if (!size){
			out=0;
			size=PAGE_SIZE;
		}
		i+=size;
	}
	return out;
}
