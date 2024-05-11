#ifndef _KERNEL_CPU_CPU_H_
#define _KERNEL_CPU_CPU_H_ 1
#include <kernel/gdt/gdt.h>
#include <kernel/memory/pmm.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>



#define CPU_KERNEL_STACK_PAGE_COUNT 8
#define CPU_PAGE_FAULT_STACK_PAGE_COUNT 16
#define CPU_INTERRUPT_STACK_PAGE_COUNT 2
#define CPU_SCHEDULER_STACK_PAGE_COUNT 1

#define CPU_HEADER_DATA ((volatile __seg_gs cpu_header_t*)NULL)



typedef volatile struct _CPU_HEADER{
	u64 index;
	u64 kernel_rsp;
	u64 user_rsp;
	struct _THREAD* current_thread;
} cpu_header_t;



typedef struct _CPU_EXTRA_DATA{
	cpu_header_t header;
	topology_t topology;
	tss_t tss;
} cpu_extra_data_t;



extern u16 cpu_count;
extern cpu_extra_data_t* cpu_extra_data;



void cpu_check_features(void);



void cpu_init_early_header(void);



void cpu_init(u16 count);



void cpu_start_all_cores(void);



#endif
