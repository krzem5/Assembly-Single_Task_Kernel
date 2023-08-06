#ifndef _KERNEL_CPU_CPU_H_
#define _KERNEL_CPU_CPU_H_ 1
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/types.h>



#define KERNEL_STACK_PAGE_COUNT 8
#define USER_STACK_PAGE_COUNT 8



extern u16 cpu_count;



static inline u64 cpu_get_stack_top(u16 core_id){
	return UMM_STACK_TOP-core_id*(USER_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
}



void cpu_init(u16 count,u64 apic_address);



void cpu_register_core(u8 apic_id);



void cpu_start_all_cores(void);



void cpu_start_program(void* start_address);



void cpu_core_stop(void);



#endif
