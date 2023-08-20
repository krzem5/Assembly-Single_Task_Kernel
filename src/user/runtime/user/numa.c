#include <user/numa.h>
#include <user/syscall.h>
#include <user/types.h>



u32 numa_node_count;



void numa_init(void){
	numa_node_count=_syscall_numa_node_count();
}
