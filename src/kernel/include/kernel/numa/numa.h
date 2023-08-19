#ifndef _KERNEL_NUMA_NUMA_H_
#define _KERNEL_NUMA_NUMA_H_ 1
#include <kernel/types.h>



#define NUMA_LOCALITY_INDEX(from,to) ((from)*numa_proximity_domain_count+(to))



extern u32 numa_proximity_domain_count;
extern u8* numa_locality_matrix;



void numa_init(u32 proximity_domain_count);



void numa_set_locality(u32 from,u32 to,u8 value);



#endif
