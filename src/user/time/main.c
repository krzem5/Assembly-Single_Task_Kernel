#include <sys/clock/clock.h>
#include <sys/io/io.h>
#include <sys/time/time.h>
#include <sys/types.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	bool raw=0;
	bool uptime=0;
	bool early_init=0;
	bool init=0;
	if (!sys_options_parse(argc,argv,"{r:raw}y{u:uptime}y{e:early}y{i:init}y",&raw,&uptime,&early_init,&init)){
		return 1;
	}
	u64 time=sys_clock_get_time_ns();
	if (early_init){
		time=sys_time_get(SYS_TIME_TYPE_EARLY_INIT);
	}
	else if (init){
		time=sys_time_get(SYS_TIME_TYPE_INIT);
	}
	else if (!uptime){
		time+=sys_time_get(SYS_TIME_TYPE_BOOT);
	}
	if (raw){
		sys_io_print("%lu\n",time);
		return 0;
	}
	sys_time_t split_time;
	sys_time_from_nanoseconds(time,&split_time);
	if (uptime||early_init||init){
		split_time.years=0;
		split_time.months=0;
		split_time.days=time/86400000000000ull;
	}
	bool force_print=0;
	if (split_time.years){
		sys_io_print("\x1b[1m%lu\x1b[0my ",split_time.years);
		force_print=1;
	}
	if (split_time.months){
		sys_io_print("\x1b[1m%lu\x1b[0mm ",split_time.months);
		force_print=1;
	}
	if (split_time.days){
		sys_io_print("\x1b[1m%lu\x1b[0md ",split_time.days);
		force_print=1;
	}
	if (force_print||split_time.hours){
		sys_io_print("\x1b[1m%lu\x1b[0mh ",split_time.hours);
		force_print=1;
	}
	if (force_print||split_time.minutes){
		sys_io_print("\x1b[1m%lu\x1b[0mm ",split_time.minutes);
		force_print=1;
	}
	if (force_print||split_time.seconds){
		sys_io_print("\x1b[1m%lu\x1b[0ms ",split_time.seconds);
		force_print=1;
	}
	if (force_print||split_time.miliseconds){
		sys_io_print("\x1b[1m%lu\x1b[0mms ",split_time.miliseconds);
		force_print=1;
	}
	if (force_print||split_time.microseconds){
		sys_io_print("\x1b[1m%lu\x1b[0mμs ",split_time.microseconds);
	}
	sys_io_print("\x1b[1m%lu\x1b[0mns\n",split_time.nanoseconds);
	return 0;
}
