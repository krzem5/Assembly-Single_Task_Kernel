#ifndef _KERNEL_CPU_CPU_H_
#define _KERNEL_CPU_CPU_H_ 1
#include <kernel/gdt/gdt.h>
#include <kernel/isr/isr.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/types.h>



#define CPU_KERNEL_STACK_PAGE_COUNT 8
#define CPU_USER_STACK_PAGE_COUNT 8

#define CPU_FLAG_PRESENT 1
#define CPU_FLAG_ONLINE 2

#define CPU_DATA ((volatile __seg_gs cpu_data_t*)NULL)



typedef struct _CPU_DATA{
	u8 index;
	u8 flags;
	u8 _padding[5];
	u64 kernel_rsp;
	u64 user_rsp;
	u64 isr_rsp;
	u64 user_func;
	u64 user_func_arg[2];
	u64 user_rsp_top;
	u32 irq_bitmap[8];
} cpu_data_t;



typedef struct _CPU_COMMON_DATA{
	tss_t tss;
	u8 isr_stack[ISR_STACK_SIZE];
} cpu_common_data_t;



extern u16 cpu_count;
extern u8 cpu_bsp_core_id;



void cpu_init(u16 count);



void cpu_register_core(u8 apic_id);



void KERNEL_NORETURN cpu_start_all_cores(void);



void cpu_core_start(u8 index,u64 start_address,u64 arg1,u64 arg2);



void KERNEL_NORETURN cpu_core_stop(void);



#endif
