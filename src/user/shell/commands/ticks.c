#include <command.h>
#include <sys/clock.h>
#include <sys/io.h>



void ticks_main(int argc,const char*const* argv){
	if (argc>1){
		printf("ticks: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("%lu\n",clock_get_ticks());
}



DECLARE_COMMAND(ticks,"ticks");
