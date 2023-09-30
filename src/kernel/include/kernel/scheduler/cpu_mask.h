#ifndef _KERNEL_SCHEDULER_CPU_MASK_H_
#define _KERNEL_SCHEDULER_CPU_MASK_H_ 1
#include <kernel/types.h>



typedef struct _CPU_MASK{
	_Atomic u64 bitmap[0];
} cpu_mask_t;



extern u32 cpu_mask_size;



void cpu_mask_init(void);



cpu_mask_t* cpu_mask_new(void);



void cpu_mask_delete(cpu_mask_t* cpu_mask);



#endif
