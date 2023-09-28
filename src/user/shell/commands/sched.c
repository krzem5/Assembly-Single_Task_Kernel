#include <command.h>
#include <string.h>
#include <user/cpu.h>
#include <user/io.h>
#include <user/scheduler.h>



void sched_main(int argc,const char*const* argv){
	if (argc>1){
		printf("sched: unrecognized option '%s'\n",argv[1]);
		return;
	}
	for (u32 i=0;i<cpu_count;i++){
		scheduler_stats_t stats;
		if (!scheduler_get_stats(i,&stats)){
			goto _error;
		}
		printf("#%u:\t%lu\n",i,stats.added_thread_count);
	}
	return;
_error:
	printf("sched: unable to read scheduler stats\n");
}



DECLARE_COMMAND(sched,"sched");
