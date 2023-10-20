#ifndef _KERNEL_CPU_CPU_H_
#define _KERNEL_CPU_CPU_H_ 1
#include <kernel/gdt/gdt.h>
#include <kernel/memory/pmm.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>



#define CPU_KERNEL_STACK_PAGE_COUNT 8
#define CPU_PAGE_FAULT_STACK_PAGE_COUNT 1

#define CPU_INTERRUPT_STACK_SIZE 4096
#define CPU_SCHEDULER_STACK_SIZE 512

#define CPU_HEADER_DATA ((volatile __seg_gs cpu_header_t*)NULL)



typedef struct _CPU_HEADER{
	u8 index;
	u8 _padding[7];
	u64 kernel_rsp;
	u64 user_rsp;
	struct _THREAD* current_thread;
} cpu_header_t;



typedef struct _CPU_EXTRA_DATA{
	cpu_header_t header;
	topology_t topology;
	tss_t tss;
	u8 interrupt_stack[CPU_INTERRUPT_STACK_SIZE];
	u8 scheduler_stack[CPU_SCHEDULER_STACK_SIZE];
	u8 pf_stack[CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT];
} cpu_extra_data_t;



extern u16 cpu_count;
extern cpu_extra_data_t* cpu_extra_data;



void cpu_check_features(void);



void cpu_init(u16 count);



void cpu_start_all_cores(void);



#endif
