#include <command.h>
#include <user/io.h>
#include <user/numa.h>



void numa_main(int argc,const char*const* argv){
	if (argc>1){
		printf("numa: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("NUMA node count: %u\n",numa_node_count);
}



DECLARE_COMMAND(numa,"numa");
