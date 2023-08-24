#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "numa"



u32 numa_node_count=0;
numa_node_t* numa_nodes;
u8* numa_node_locality_matrix;



void numa_init(u32 proximity_domain_count){
	if (numa_node_count&&proximity_domain_count!=numa_node_count){
		ERROR("NUMA proximity domain count mismatch between SLIT and SRAT");
		return;
	}
	LOG("Initializing NUMA...");
	INFO("Proximity domain count: %u",proximity_domain_count);
	numa_node_count=proximity_domain_count;
	numa_nodes=kmm_alloc(numa_node_count*sizeof(numa_node_t));
	numa_node_locality_matrix=kmm_alloc(numa_node_count*numa_node_count*sizeof(u8));
	for (u32 i=0;i<numa_node_count;i++){
		(numa_nodes+i)->index=i;
		(numa_nodes+i)->cpu_count=0;
		(numa_nodes+i)->memory_range_count=0;
		(numa_nodes+i)->cpus=NULL;
		(numa_nodes+i)->memory_ranges=NULL;
		for (u32 j=0;j<numa_node_count;j++){
			numa_node_locality_matrix[NUMA_LOCALITY_INDEX(i,j)]=0;
		}
	}
}



void numa_set_locality(u32 from,u32 to,u8 value){
	if (from>=numa_node_count||to>=numa_node_count){
		ERROR("NUMA node index out-of-range");
		return;
	}
	numa_node_locality_matrix[NUMA_LOCALITY_INDEX(from,to)]=value;
}



void numa_add_cpu(u32 node_index,u8 apic_id,u32 sapic_eid){
	if (node_index>=numa_node_count){
		ERROR("NUMA node index out-of-range");
		return;
	}
	numa_cpu_t* cpu=kmm_alloc(sizeof(numa_cpu_t));
	cpu->next=(numa_nodes+node_index)->cpus;
	cpu->apic_id=apic_id;
	cpu->sapic_eid=sapic_eid;
	(numa_nodes+node_index)->cpu_count++;
	(numa_nodes+node_index)->cpus=cpu;
}



void numa_add_memory_range(u32 node_index,u64 base_address,u64 length,_Bool hot_pluggable){
	if (node_index>=numa_node_count){
		ERROR("NUMA node index out-of-range");
		return;
	}
	numa_memory_range_t* memory_range=kmm_alloc(sizeof(numa_memory_range_t));
	memory_range->next=(numa_nodes+node_index)->memory_ranges;
	memory_range->base_address=base_address;
	memory_range->length=length;
	memory_range->hot_pluggable=hot_pluggable;
	(numa_nodes+node_index)->memory_range_count++;
	(numa_nodes+node_index)->memory_ranges=memory_range;
}
