#include <sys2/clock/clock.h>
#include <sys2/io/io.h>
#include <sys2/time/time.h>
#include <sys2/types.h>
#include <sys2/util/options.h>



int main(int argc,const char** argv){
	_Bool raw=0;
	_Bool uptime=0;
	sys2_option_t options[]={
		{
			.short_name='r',
			.long_name="raw",
			.var_type=SYS2_OPTION_VAR_TYPE_SWITCH,
			.flags=0,
			.var_switch=&raw
		},
		{
			.short_name='u',
			.long_name="uptime",
			.var_type=SYS2_OPTION_VAR_TYPE_SWITCH,
			.flags=0,
			.var_switch=&uptime
		},
		{
			.var_type=SYS2_OPTION_VAR_TYPE_LAST
		}
	};
	if (!sys2_options_parse(argc,argv,options)){
		return 1;
	}
	u64 time=sys2_clock_get_time_ns();
	if (!uptime){
		time+=sys2_time_get_boot_offset();
	}
	if (raw){
		sys2_io_print("%lu\n",time);
		return 0;
	}
	sys2_time_t split_time;
	sys2_time_from_nanoseconds(time,&split_time);
	if (uptime){
		split_time.years=0;
		split_time.months=0;
		split_time.days=time/86400000000000ull;
	}
	_Bool force_print=0;
	if (split_time.years){
		sys2_io_print("\x1b[1m%lu\x1b[0my ",split_time.years);
		force_print=1;
	}
	if (split_time.months){
		sys2_io_print("\x1b[1m%lu\x1b[0mm ",split_time.months);
		force_print=1;
	}
	if (split_time.days){
		sys2_io_print("\x1b[1m%lu\x1b[0md ",split_time.days);
		force_print=1;
	}
	if (force_print||split_time.hours){
		sys2_io_print("\x1b[1m%lu\x1b[0mh ",split_time.hours);
		force_print=1;
	}
	if (force_print||split_time.minutes){
		sys2_io_print("\x1b[1m%lu\x1b[0mm ",split_time.minutes);
		force_print=1;
	}
	if (force_print||split_time.seconds){
		sys2_io_print("\x1b[1m%lu\x1b[0ms ",split_time.seconds);
		force_print=1;
	}
	if (force_print||split_time.miliseconds){
		sys2_io_print("\x1b[1m%lu\x1b[0mms ",split_time.miliseconds);
		force_print=1;
	}
	if (force_print||split_time.microseconds){
		sys2_io_print("\x1b[1m%lu\x1b[0mμs ",split_time.microseconds);
	}
	sys2_io_print("\x1b[1m%lu\x1b[0mns\n",split_time.nanoseconds);
	return 0;
}
