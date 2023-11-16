#include <command.h>
#include <string.h>
#include <sys/clock.h>
#include <sys/io.h>
#include <sys/time.h>
#include <sys/types.h>



void time_main(int argc,const char*const* argv){
	_Bool raw=0;
	_Bool uptime=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-r")){
			raw=1;
		}
		else if (string_equal(argv[i],"-u")){
			uptime=1;
		}
		else{
			printf("time: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	u64 time=sys_clock_get_time();
	if (!uptime){
		time+=sys_time_get_boot_offset();
	}
	if (raw){
		printf("%lu\n",time);
		return;
	}
	sys_time_t split_time;
	sys_time_from_nanoseconds(time,&split_time);
	if (uptime){
		split_time.years=0;
		split_time.months=0;
		split_time.days=time/86400000000000ull;
	}
	_Bool force_print=0;
	if (split_time.years){
		printf("\x1b[1m%lu\x1b[0my ",split_time.years);
		force_print=1;
	}
	if (split_time.months){
		printf("\x1b[1m%lu\x1b[0mm ",split_time.months);
		force_print=1;
	}
	if (split_time.days){
		printf("\x1b[1m%lu\x1b[0md ",split_time.days);
		force_print=1;
	}
	if (force_print||split_time.hours){
		printf("\x1b[1m%lu\x1b[0mh ",split_time.hours);
		force_print=1;
	}
	if (force_print||split_time.minutes){
		printf("\x1b[1m%lu\x1b[0mm ",split_time.minutes);
		force_print=1;
	}
	if (force_print||split_time.seconds){
		printf("\x1b[1m%lu\x1b[0ms ",split_time.seconds);
		force_print=1;
	}
	if (force_print||split_time.miliseconds){
		printf("\x1b[1m%lu\x1b[0mms ",split_time.miliseconds);
		force_print=1;
	}
	if (force_print||split_time.microseconds){
		printf("\x1b[1m%lu\x1b[0mμs ",split_time.microseconds);
	}
	printf("\x1b[1m%lu\x1b[0mns\n",split_time.nanoseconds);
}



DECLARE_COMMAND(time,"time [-r] [-u]");
