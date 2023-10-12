#include <user/numa.h>
#include <user/syscall.h>
#include <user/types.h>



u32 numa_get_node_count(void){
	return _syscall_numa_get_node_count();
}



_Bool numa_get_node(u32 index,numa_node_t* out){
	return _syscall_numa_get_node(index,out,sizeof(numa_node_t));
}



_Bool numa_get_node_cpu(u32 index,u32 cpu_index,numa_cpu_t* out){
	return _syscall_numa_get_node_cpu(index,cpu_index,out,sizeof(numa_cpu_t));
}



_Bool numa_get_node_memory_range(u32 index,u32 memory_range_index,numa_memory_range_t* out){
	return _syscall_numa_get_node_memory_range(index,memory_range_index,out,sizeof(numa_memory_range_t));
}



_Bool numa_get_locality(u32 offset,u8* buffer,u32 length){
	return _syscall_numa_get_locality(offset,buffer,length);
}
