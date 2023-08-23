#include <command.h>
#include <user/io.h>
#include <user/numa.h>



void numa_main(int argc,const char*const* argv){
	if (argc>1){
		printf("numa: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("NUMA node count: \x1b[1m%u\x1b[0m\nNUMA locality matrix:\n",numa_node_count);
	for (u32 i=0;i<numa_node_count;i++){
		for (u32 j=0;j<numa_node_count;j++){
			printf("[%u,%u] \x1b[1m%u\x1b[0m\n",i,j,numa_node_locality_matrix[i*numa_node_count+j]);
		}
	}
	*((char*)0)=0;
}



DECLARE_COMMAND(numa,"numa");
