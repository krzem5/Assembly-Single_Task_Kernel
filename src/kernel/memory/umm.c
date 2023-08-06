#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "umm"



static u64 _umm_user_stacks_base;
static u64 _umm_user_stacks_length;
static u64 _umm_cpu_common_data_base;
static u64 _umm_cpu_common_data_length;
static u64 _umm_idt_data_base;

u64 umm_highest_free_address;



void umm_init_pagemap(vmm_pagemap_t* pagemap){
	LOG("Initializing user pagemap at %p...",pagemap->toplevel);
	INFO("Mapping %v from %p to %p...",kernel_get_end()-kernel_get_common_start(),kernel_get_common_start(),kernel_get_common_start()+kernel_get_offset());
	vmm_map_pages(pagemap,kernel_get_common_start(),kernel_get_common_start()+kernel_get_offset(),VMM_PAGE_FLAG_PRESENT,pmm_align_up_address(kernel_get_end()-kernel_get_common_start())>>PAGE_SIZE_SHIFT);
	INFO("Mapping %v from %p to %p...",_umm_cpu_common_data_length<<PAGE_SIZE_SHIFT,_umm_cpu_common_data_base,VMM_TRANSLATE_ADDRESS(_umm_cpu_common_data_base));
	vmm_map_pages(pagemap,_umm_cpu_common_data_base,_umm_cpu_common_data_base+vmm_address_offset,VMM_PAGE_FLAG_PRESENT,_umm_cpu_common_data_length);
	INFO("Mapping %v from %p to %p...",_umm_user_stacks_length<<PAGE_SIZE_SHIFT,_umm_user_stacks_base,UMM_STACK_TOP-(_umm_user_stacks_length<<PAGE_SIZE_SHIFT));
	vmm_map_pages(pagemap,_umm_user_stacks_base,UMM_STACK_TOP-(_umm_user_stacks_length<<PAGE_SIZE_SHIFT),VMM_PAGE_FLAG_PRESENT|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_USER,_umm_user_stacks_length);
	INFO("Mapping %v from %p to %p...",PAGE_SIZE,_umm_idt_data_base,_umm_idt_data_base+vmm_address_offset);
	vmm_map_page(pagemap,_umm_idt_data_base,_umm_idt_data_base+vmm_address_offset,VMM_PAGE_FLAG_PRESENT);
}



void umm_set_user_stacks(u64 base,u64 length){
	_umm_user_stacks_base=base;
	_umm_user_stacks_length=length;
	umm_highest_free_address=UMM_STACK_TOP-(length<<PAGE_SIZE_SHIFT);
}



void umm_set_cpu_common_data(u64 base,u64 length){
	_umm_cpu_common_data_base=base;
	_umm_cpu_common_data_length=length;
}



void umm_set_idt_data(u64 base){
	_umm_idt_data_base=base;
}
