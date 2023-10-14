#include <user/partition.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool partition_get(u32 index,partition_t* out){
	return _syscall_partition_get(index,out,sizeof(partition_t));
}
