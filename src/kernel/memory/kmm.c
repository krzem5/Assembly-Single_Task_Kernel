#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "kmm"



static u64 _kmm_top=0;
static u64 _kmm_max_top=0;
static _Bool _kmm_buffer_not_ended=0;



static void _resize_stack(void){
	while (_kmm_top>_kmm_max_top){
		vmm_map_page(&vmm_kernel_pagemap,pmm_alloc(1,PMM_COUNTER_KMM),_kmm_max_top,VMM_PAGE_FLAG_PRESENT|VMM_PAGE_FLAG_READWRITE);
		_kmm_max_top+=PAGE_SIZE;
	}
}



void kmm_init(void){
	LOG("Initializing kernel memory manager...");
	_kmm_top=pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset());
	_kmm_max_top=pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset());
}



void* kmm_allocate(u32 size){
	void* out=(void*)_kmm_top;
	_kmm_top+=(size+7)&0xfffffffffffffff8ull;
	_resize_stack();
	return out;
}



void* kmm_allocate_buffer(void){
	if (_kmm_buffer_not_ended){
		ERROR("Buffer already in use");
		for (;;);
	}
	return (void*)_kmm_top;
}



void kmm_grow_buffer(u32 size){
	_kmm_top+=size;
	_resize_stack();
}



void kmm_end_buffer(void){
	_kmm_top=(_kmm_top+7)&0xfffffffffffffff8ull;
	_resize_stack();
}
