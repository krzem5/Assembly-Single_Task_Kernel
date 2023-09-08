#ifndef _KERNEL_TOPOLOGY_TOPOLOGY_H_
#define _KERNEL_TOPOLOGY_TOPOLOGY_H_ 1
#include <kernel/types.h>



typedef struct _TOPOLOGY{
	u32 thread;
	u32 core;
	u32 chip;
} topology_t;



void topology_compute(u8 apic_id,topology_t* out);



u64 topology_get_cpu_bits(void);



#endif
