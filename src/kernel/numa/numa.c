#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "numa"



u32 numa_proximity_domain_count=0;
u8* numa_locality_matrix;



void numa_init(u32 proximity_domain_count){
	if (numa_proximity_domain_count&&proximity_domain_count!=numa_proximity_domain_count){
		ERROR("NUMA proximity domain count mismatch between SLIT and SRAT");
		return;
	}
	LOG("Initializing NUMA...");
	INFO("Proximity domain count: %u",proximity_domain_count);
	numa_proximity_domain_count=proximity_domain_count;
	numa_locality_matrix=kmm_allocate(numa_proximity_domain_count*numa_proximity_domain_count*sizeof(u8));
	for (u32 i=0;i<numa_proximity_domain_count*numa_proximity_domain_count;i++){
		numa_locality_matrix[i]=0;
	}
}



void numa_set_locality(u32 from,u32 to,u8 value){
	if (from>=numa_proximity_domain_count||to>=numa_proximity_domain_count){
		ERROR("NUMA locality pairt out-of-range");
		return;
	}
	numa_locality_matrix[NUMA_LOCALITY_INDEX(from,to)]=value;
}
