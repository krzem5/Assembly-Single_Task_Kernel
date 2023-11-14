#include <command.h>
#include <core/io.h>



void numa_main(int argc,const char*const* argv){
	if (argc>1){
		printf("numa: unrecognized option '%s'\n",argv[1]);
		return;
	}
	// u32 numa_node_count=numa_get_node_count();
	// printf("NUMA nodes: (\x1b[1m%u\x1b[0m)\n",numa_node_count);
	// for (u32 i=0;i<numa_node_count;i++){
	// 	numa_node_t node;
	// 	if (!numa_get_node(i,&node)){
	// 		continue;
	// 	}
	// 	printf("[\x1b[1m%u\x1b[0m]:\n  CPUs: (\x1b[1m%u\x1b[0m)\n",i,node.cpu_count);
	// 	for (u32 j=0;j<node.cpu_count;j++){
	// 		numa_cpu_t cpu;
	// 		if (!numa_get_node_cpu(i,j,&cpu)){
	// 			continue;
	// 		}
	// 		printf("    #\x1b[1m%u\x1b[0m\n",cpu.apic_id);
	// 	}
	// 	printf("  Memory ranges: (\x1b[1m%u\x1b[0m)\n",node.memory_range_count);
	// 	for (u32 j=0;j<node.memory_range_count;j++){
	// 		numa_memory_range_t memory_range;
	// 		if (!numa_get_node_memory_range(i,j,&memory_range)){
	// 			continue;
	// 		}
	// 		printf("    %p - %p (\x1b[1m%v\x1b[0m)%s\n",memory_range.base_address,memory_range.base_address+memory_range.length,memory_range.length,(memory_range.hot_pluggable?" [Hot-pluggable]":""));
	// 	}
	// 	printf("  Locality:\n");
	// 	for (u32 j=0;j<numa_node_count;j++){
	// 		u8 distance;
	// 		if (!numa_get_locality(i*numa_node_count+j,&distance,1)){
	// 			distance=0;
	// 		}
	// 		printf("    [%u -> %u]: \x1b[1m%u\x1b[0m\n",i,j,distance);
	// 	}
	// }
}



DECLARE_COMMAND(numa,"numa");
