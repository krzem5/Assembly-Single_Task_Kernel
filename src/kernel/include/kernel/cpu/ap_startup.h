#ifndef _KERNEL_CPU_AP_STARTUP_H_
#define _KERNEL_CPU_AP_STARTUP_H_ 1
#include <kernel/types.h>



#define CPU_AP_STARTUP_MEMORY_ADDRESS 0x2000



void cpu_ap_startup_init(void* cpu_stack_list,void* cpu_extra_data_list);



#endif
