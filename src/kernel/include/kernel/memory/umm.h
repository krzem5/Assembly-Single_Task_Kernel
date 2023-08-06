#ifndef _KERNEL_MEMORY_UMM_H_
#define _KERNEL_MEMORY_UMM_H_ 1
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



#define UMM_STACK_TOP 0x0000008000000000ull



extern u64 umm_highest_free_address;



void umm_init_pagemap(vmm_pagemap_t* pagemap);



void umm_set_user_stacks(u64 base,u64 length);



void umm_set_cpu_common_data(u64 base,u64 length);



void umm_set_idt_data(u64 base);



#endif
