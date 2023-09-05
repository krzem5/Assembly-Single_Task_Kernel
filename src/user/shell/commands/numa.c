#include <command.h>
#include <user/io.h>
#include <user/numa.h>



void numa_main(int argc,const char*const* argv){
	if (argc>1){
		printf("numa: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("NUMA nodes: (\x1b[1m%u\x1b[0m)\n",numa_node_count);
	for (u32 i=0;i<numa_node_count;i++){
		const numa_node_t* numa=numa_nodes+i;
		printf("[\x1b[1m%u\x1b[0m]:\n  CPUs: (\x1b[1m%u\x1b[0m)\n",i,numa->cpu_count);
		for (u32 j=0;j<numa->cpu_count;j++){
			printf("    #\x1b[1m%u\x1b[0m\n",(numa->cpus+j)->apic_id);
		}
		printf("  Memory ranges: (\x1b[1m%u\x1b[0m)\n",numa->memory_range_count);
		for (u32 j=0;j<numa->memory_range_count;j++){
			const numa_memory_range_t* memory_range=numa->memory_ranges+j;
			printf("    %p - %p (\x1b[1m%v\x1b[0m)%s\n",memory_range->base_address,memory_range->base_address+memory_range->length,memory_range->length,(memory_range->hot_pluggable?" [Hot-pluggable]":""));
		}
		printf("  Locality:\n");
		for (u32 j=0;j<numa_node_count;j++){
			printf("    [%u -> %u]: \x1b[1m%u\x1b[0m\n",i,j,numa_node_locality_matrix[i*numa_node_count+j]);
		}
	}
}



DECLARE_COMMAND(numa,"numa");
