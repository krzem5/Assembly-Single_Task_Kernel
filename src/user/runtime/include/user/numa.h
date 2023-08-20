#ifndef _USER_NUMA_H_
#define _USER_NUMA_H_ 1
#include <user/types.h>



#define MAX_NUMA_NODES 32



typedef struct _NUMA_NODE{
} numa_node_t;



extern numa_node_t numa_nodes[MAX_NUMA_NODES];
extern u32 numa_node_count;
extern u8 numa_node_locality_matrix[MAX_NUMA_NODES*MAX_NUMA_NODES];



void numa_init(void);



#endif
