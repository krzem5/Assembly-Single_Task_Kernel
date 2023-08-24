#ifndef _USER_NUMA_H_
#define _USER_NUMA_H_ 1
#include <user/types.h>



typedef struct _NUMA_CPU{
	u8 apic_id;
	u32 sapic_eid;
} numa_cpu_t;



typedef struct _NUMA_MEMORY_RANGE{
	u64 base_address;
	u64 length;
	_Bool hot_pluggable;
} numa_memory_range_t;



typedef struct _NUMA_NODE{
	u32 index;
	u32 cpu_count;
	u32 memory_range_count;
	const numa_cpu_t* cpus;
	const numa_memory_range_t* memory_ranges;
} numa_node_t;



extern u32 numa_node_count;
extern const numa_node_t* numa_nodes;
extern const u8* numa_node_locality_matrix;



#endif
