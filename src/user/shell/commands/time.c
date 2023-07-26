#include <command.h>
#include <user/clock.h>
#include <user/io.h>



void time_main(int argc,const char*const* argv){
	if (argc>1){
		printf("time: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("%lu\n",clock_get_time());
}



DECLARE_COMMAND(time,"time");
