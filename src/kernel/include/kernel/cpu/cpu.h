#ifndef _KERNEL_CPU_CPU_H_
#define _KERNEL_CPU_CPU_H_ 1
#include <kernel/gdt/gdt.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>



#define CPU_KERNEL_STACK_PAGE_COUNT 8
#define CPU_PAGE_FAULT_STACK_PAGE_COUNT 1

#define CPU_INTERRUPT_STACK_SIZE 4096
#define CPU_SCHEDULER_STACK_SIZE 512

#define CPU_FLAG_PRESENT 1
#define CPU_FLAG_ONLINE 2

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
	u8 flags;
	topology_t topology;
	tss_t tss;
	u8 interrupt_stack[CPU_INTERRUPT_STACK_SIZE];
	u8 scheduler_stack[CPU_SCHEDULER_STACK_SIZE];
} cpu_extra_data_t;



extern cpu_extra_data_t* cpu_extra_data;
extern u16 cpu_count;
extern u8 cpu_bsp_core_id;



void cpu_check_features(void);



void cpu_init(u16 count);



void cpu_register_core(u8 apic_id);



void cpu_start_all_cores(void);



#endif
