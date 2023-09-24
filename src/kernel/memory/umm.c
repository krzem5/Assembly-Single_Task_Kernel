#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "umm"



#define UMM_CONSTANT_STACK_START 0xfffff00000000000ull



PMM_DECLARE_COUNTER(UMM);



static lock_t _umm_stack_lock=LOCK_INIT_STRUCT;
static u64 _umm_stack_top=UMM_CONSTANT_STACK_START;
static u64 _umm_stack_max_top=UMM_CONSTANT_STACK_START;



void* umm_alloc(u32 size){
	lock_acquire_exclusive(&_umm_stack_lock);
	void* out=(void*)_umm_stack_top;
	_umm_stack_top+=(size+7)&0xfffffffffffffff8ull;
	while (_umm_stack_top>_umm_stack_max_top){
		u64 page=pmm_alloc(1,PMM_COUNTER_UMM,0);
		vmm_map_page(&vmm_kernel_pagemap,page,_umm_stack_max_top,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_PRESENT);
		_umm_stack_max_top+=PAGE_SIZE;
	}
	lock_release_exclusive(&_umm_stack_lock);
	return out;
}
