#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "mmap"



u64 mmap_alloc(u64 length,u8 flags){
	// u64 size=PAGE_SIZE;
	u64 page_flags=VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
	if (flags&MMAP_FLAG_EXTRA_LARGE){
		// size=EXTRA_LARGE_PAGE_SIZE;
		length=pmm_align_up_address_extra_large(length);
		page_flags|=VMM_PAGE_FLAG_EXTRA_LARGE;
	}
	else if (flags&MMAP_FLAG_LARGE){
		// size=LARGE_PAGE_SIZE;
		length=pmm_align_up_address_large(length);
		page_flags|=VMM_PAGE_FLAG_LARGE;
	}
	else{
		length=pmm_align_up_address(length);
	}
	u64 out=vmm_memory_map_reserve(&(CPU_DATA->scheduler->current_thread->process->mmap),0,length);
	vmm_reserve_pages(&(CPU_DATA->scheduler->current_thread->process->user_pagemap),out,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE,length>>PAGE_SIZE_SHIFT);
	return out;
}



_Bool mmap_dealloc(u64 address,u64 length){
	if (vmm_memory_map_release(&(CPU_DATA->scheduler->current_thread->process->mmap),address,pmm_align_up_address(length))){
		vmm_release_pages(&(CPU_DATA->scheduler->current_thread->process->user_pagemap),address,pmm_align_up_address(length)>>PAGE_SIZE_SHIFT);
		return 1;
	}
	return 0;
}
