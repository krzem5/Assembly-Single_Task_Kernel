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
	printf("CPU count: %u\nCPU frequency: %lu MHz\n",cpu_count,(clock_cpu_frequency+500000)/1000000);
}



DECLARE_COMMAND(cpu,"cpu");
