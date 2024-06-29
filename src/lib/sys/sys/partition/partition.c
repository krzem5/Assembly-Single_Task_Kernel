#include <sys/drive/drive.h>
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



SYS_PUBLIC sys_error_t sys_partition_table_descriptor_format(sys_drive_t drive,sys_partition_table_descriptor_t partition_table_descriptor){
	return _sys_syscall_partition_table_descriptor_format(drive,partition_table_descriptor);
}



SYS_PUBLIC sys_partition_table_descriptor_t sys_partition_table_descriptor_iter_start(void){
	return _sys_syscall_partition_table_descriptor_get_next(0);
}



SYS_PUBLIC sys_partition_table_descriptor_t sys_partition_table_descriptor_iter_next(sys_partition_table_descriptor_t partition_table_descriptor){
	return _sys_syscall_partition_table_descriptor_get_next(partition_table_descriptor);
}



SYS_PUBLIC sys_error_t sys_partition_table_descriptor_get_data(sys_partition_table_descriptor_t partition_table_descriptor,sys_partition_table_descriptor_data_t* out){
	return _sys_syscall_partition_table_descriptor_get_data(partition_table_descriptor,out,sizeof(sys_partition_table_descriptor_data_t));
}
