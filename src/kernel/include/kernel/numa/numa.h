#ifndef _KERNEL_NUMA_NUMA_H_
#define _KERNEL_NUMA_NUMA_H_ 1
#include <kernel/types.h>



#define NUMA_LOCALITY_INDEX(from,to) ((from)*numa_node_count+(to))



typedef struct _NUMA_CPU{
	struct _NUMA_CPU* next;
	u8 apic_id;
	u32 sapic_eid;
} numa_cpu_t;



typedef struct _NUMA_MEMORY_RANGE{
	struct _NUMA_MEMORY_RANGE* next;
	u64 base_address;
	u64 length;
	bool hot_pluggable;
} numa_memory_range_t;



typedef struct _NUMA_NODE{
	u32 index;
	u32 cpu_count;
	u32 memory_range_count;
	numa_cpu_t* cpus;
	numa_memory_range_t* memory_ranges;
} numa_node_t;



extern u32 numa_node_count;
extern numa_node_t* numa_nodes;
extern u8* numa_node_locality_matrix;



void numa_init(u32 proximity_domain_count,u32 cpu_count,u32 memory_range_count);



void numa_set_locality(u32 from,u32 to,u8 value);



void numa_add_cpu(u32 node_index,u8 apic_id,u32 sapic_eid);



void numa_add_memory_range(u32 node_index,u64 base_address,u64 length,bool hot_pluggable);



#endif
