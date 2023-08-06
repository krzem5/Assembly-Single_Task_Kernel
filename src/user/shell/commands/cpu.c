#include <command.h>
#include <user/clock.h>
#include <user/cpu.h>
#include <user/fs.h>
#include <user/io.h>



void cpu_main(int argc,const char*const* argv){
	if (argc>1){
		printf("cpu: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("CPU count: \x1b[1m%u\x1b[0m\nCPU frequency: \x1b[1m%lu MHz\x1b[0m\n",cpu_count,(clock_cpu_frequency+500000)/1000000);
}



DECLARE_COMMAND(cpu,"cpu");
