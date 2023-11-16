#include <command.h>
#include <string.h>
#include <sys/clock.h>
#include <sys/cpu.h>
#include <sys/io.h>
// #include <sys/scheduler.h>



void sched_main(int argc,const char*const* argv){
	if (argc>1){
		printf("sched: unrecognized option '%s'\n",argv[1]);
		return;
	}
	u32 cpu_count=sys_cpu_get_count();
	for (u32 i=0;i<cpu_count;i++){
		// scheduler_stats_t stats;
		// if (!scheduler_get_stats(i,&stats)){
		// 	goto _error_stats;
		// }
		// scheduler_timers_t timers;
		// if (!scheduler_get_timers(i,&timers)){
		// 	goto _error_timers;
		// }
		// printf("#%u:\t%lu\t%lu\t%lu\t%lu\t%lu\n",i,stats.added_thread_count,clock_ticks_to_time(timers.timer_user)/1000000ull,clock_ticks_to_time(timers.timer_kernel)/1000000ull,clock_ticks_to_time(timers.timer_scheduler)/1000000ull,clock_ticks_to_time(timers.timer_none)/1000000ull);
	}
// 	return;
// _error_stats:
// 	printf("sched: unable to read scheduler stats\n");
// 	return;
// _error_timers:
// 	printf("sched: unable to read scheduler timers\n");
}



DECLARE_COMMAND(sched,"sched");
