#include <sys/error/error.h>
#include <sys/fs/fs.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_fs_t sys_fs_iter_start(void){
	return _sys_syscall_fs_get_next(0);
}



SYS_PUBLIC sys_fs_t sys_fs_iter_next(sys_fs_t fs){
	return _sys_syscall_fs_get_next(fs);
}



SYS_PUBLIC sys_error_t sys_fs_get_data(sys_fs_t fs,sys_fs_data_t* out){
	return _sys_syscall_fs_get_data(fs,out,sizeof(sys_fs_data_t));
}



SYS_PUBLIC sys_error_t sys_fs_mount(sys_fs_t fs,const char* path){
	return _sys_syscall_fs_mount(fs,path);
}

