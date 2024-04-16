#include <sys/drive/drive.h>
#include <sys/error/error.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_drive_t sys_drive_iter_start(void){
	return _sys_syscall_drive_get_next(0);
}



SYS_PUBLIC sys_drive_t sys_drive_iter_next(sys_drive_t drive){
	return _sys_syscall_drive_get_next(drive);
}



SYS_PUBLIC sys_error_t sys_drive_get_data(sys_drive_t drive,sys_drive_data_t* out){
	return _sys_syscall_drive_get_data(drive,out,sizeof(sys_drive_data_t));
}
