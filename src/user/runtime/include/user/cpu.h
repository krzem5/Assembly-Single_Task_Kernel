#ifndef _USER_CPU_H_
#define _USER_CPU_H_ 1
#include <user/types.h>



extern u32 cpu_count;
extern u32 cpu_bsp_id;



void cpu_core_start(u32 core,void* func,void* arg);



void cpu_core_stop(void);



#endif
