#include <sys/error/error.h>
#include <sys/partition/partition.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_partition_t sys_partition_iter_start(void){
	return _sys_syscall_partition_get_next(0);
}



SYS_PUBLIC sys_partition_t sys_partition_iter_next(sys_partition_t partition){
	return _sys_syscall_partition_get_next(partition);
}



SYS_PUBLIC sys_error_t sys_partition_get_data(sys_partition_t partition,sys_partition_data_t* out){
	return _sys_syscall_partition_get_data(partition,out,sizeof(sys_partition_data_t));
}
