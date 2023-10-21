#include <user/partition.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool partition_get_data(u64 handle,partition_data_t* out){
	return _syscall_partition_get_data(handle,out,sizeof(partition_data_t));
}
