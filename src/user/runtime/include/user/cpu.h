#ifndef _USER_CPU_H_
#define _USER_CPU_H_ 1
#include <user/types.h>



typedef struct _CPU{
	u32 domain;
	u32 chip;
	u32 core;
	u32 thread;
} cpu_t;



extern u32 cpu_count;
extern const cpu_t* cpus;



#endif
