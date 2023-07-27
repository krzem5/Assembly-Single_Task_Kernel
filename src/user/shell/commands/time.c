#include <command.h>
#include <string.h>
#include <user/clock.h>
#include <user/io.h>



void time_main(int argc,const char*const* argv){
	_Bool raw=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-r")){
			raw=1;
		}
		else{
			printf("time: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	u64 time=clock_get_time();
	if (raw){
		printf("%lu\n",time);
		return;
	}
	u64 days=time/86400000000000ull;
	u64 hours=(time/3600000000000ull)%24;
	u64 minutes=(time/60000000000ull)%60;
	u64 seconds=(time/1000000000ull)%60;
	u64 miliseconds=(time/1000000ull)%1000;
	u64 microseconds=(time/1000ull)%1000;
	u64 nanoseconds=time%1000;
	_Bool force_print=0;
	if (days){
		printf("\x1b[1m%lu\x1b[0md ",days);
		force_print=1;
	}
	if (force_print||hours){
		printf("\x1b[1m%lu\x1b[0mh ",hours);
		force_print=1;
	}
	if (force_print||minutes){
		printf("\x1b[1m%lu\x1b[0mm ",minutes);
		force_print=1;
	}
	if (force_print||seconds){
		printf("\x1b[1m%lu\x1b[0ms ",seconds);
		force_print=1;
	}
	if (force_print||miliseconds){
		printf("\x1b[1m%lu\x1b[0mms ",miliseconds);
		force_print=1;
	}
	if (force_print||microseconds){
		printf("\x1b[1m%lu\x1b[0mμs ",microseconds);
	}
	printf("\x1b[1m%lu\x1b[0mns\n",nanoseconds);
}



DECLARE_COMMAND(time,"time [-r]");
