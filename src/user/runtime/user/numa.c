#include <user/numa.h>
#include <user/syscall.h>
#include <user/types.h>



numa_node_t numa_nodes[MAX_NUMA_NODES];
u32 numa_node_count;
u8 numa_node_locality_matrix[MAX_NUMA_NODES*MAX_NUMA_NODES];



void numa_init(void){
	numa_node_count=_syscall_numa_node_count();
	for (u32 i=0;i<numa_node_count;i++){
		for (u32 j=0;j<numa_node_count;j++){
			numa_node_locality_matrix[i*numa_node_count+j]=_syscall_numa_node_locality(i,j);
		}
	}
}
