#ifndef _USER_NUMA_H_
#define _USER_NUMA_H_ 1
#include <user/types.h>



typedef struct _NUMA_NODE{
	u32 index;
	u32 cpu_count;
	u32 memory_range_count;
} numa_node_t;



typedef struct _NUMA_CPU{
	u32 numa_index;
	u32 cpu_index;
	u8 apic_id;
	u32 sapic_eid;
} numa_cpu_t;



typedef struct _NUMA_MEMORY_RANGE{
	u32 numa_index;
	u32 memory_range_index;
	u64 base_address;
	u64 length;
	_Bool hot_pluggable;
} numa_memory_range_t;



u32 numa_get_node_count(void);



_Bool numa_get_node(u32 index,numa_node_t* out);



_Bool numa_get_node_cpu(u32 index,u32 cpu_index,numa_cpu_t* out);



_Bool numa_get_node_memory_range(u32 index,u32 memory_range_index,numa_memory_range_t* out);



_Bool numa_get_locality(u32 offset,u8* buffer,u32 length);



#endif
