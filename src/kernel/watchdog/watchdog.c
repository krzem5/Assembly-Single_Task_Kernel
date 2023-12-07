#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "watchdog"



#define MAX_WATCHDOG_TIME 500000000



static CPU_LOCAL_DATA(KERNEL_ATOMIC u64,_watchdog_timers);



void watchdog_update(void){
	u64 time=clock_get_time();
	*CPU_LOCAL(_watchdog_timers)=time+MAX_WATCHDOG_TIME;
	for (u32 i=0;i<cpu_count;i++){
		if (*CPU_LOCAL(_watchdog_timers)<time){
			ERROR("%u",i);
		}
	}
}
