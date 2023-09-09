#ifndef _USER_CPU_H_
#define _USER_CPU_H_ 1
#include <user/types.h>



typedef struct _CPU{
	u8 apic_id;
	u8 flags;
	u32 domain;
	u32 chip;
	u32 core;
	u32 thread;
} cpu_t;



extern u32 cpu_count;
extern u32 cpu_bsp_id;
extern const cpu_t* cpus;



static inline const __seg_gs cpu_t* cpu_current(void){
	return NULL;
}



void cpu_core_start(u32 core,void* func,void* arg);



void cpu_core_stop(void);



#endif
