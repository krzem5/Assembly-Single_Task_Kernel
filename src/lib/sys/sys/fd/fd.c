#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_fd_t sys_fd_open(sys_fd_t fd,const char* path,u32 flags){
	return _sys_syscall_fd_open(fd,path,flags);
}



SYS_PUBLIC sys_error_t sys_fd_close(sys_fd_t fd){
	return _sys_syscall_fd_close(fd);
}



SYS_PUBLIC u64 sys_fd_read(sys_fd_t fd,void* buffer,u64 size,u32 flags){
	return _sys_syscall_fd_read(fd,buffer,size,flags);
}



SYS_PUBLIC u64 sys_fd_write(sys_fd_t fd,const void* buffer,u64 size,u32 flags){
	return _sys_syscall_fd_write(fd,buffer,size,flags);
}



SYS_PUBLIC u64 sys_fd_seek(sys_fd_t fd,u64 offset,u32 type){
	return _sys_syscall_fd_seek(fd,offset,type);
}



SYS_PUBLIC u64 sys_fd_resize(sys_fd_t fd,u64 size,u32 flags){
	return _sys_syscall_fd_resize(fd,size,flags);
}



SYS_PUBLIC sys_error_t sys_fd_stat(sys_fd_t fd,sys_fd_stat_t* out){
	return _sys_syscall_fd_stat(fd,out,sizeof(sys_fd_stat_t));
}



SYS_PUBLIC sys_fd_t sys_fd_dup(sys_fd_t fd,u32 flags){
	return _sys_syscall_fd_dup(fd,flags);
}



SYS_PUBLIC sys_error_t sys_fd_path(sys_fd_t fd,char* path,u32 size){
	return _sys_syscall_fd_path(fd,path,size);
}



SYS_PUBLIC sys_fd_iterator_t sys_fd_iter_start(sys_fd_t fd){
	return _sys_syscall_fd_iter_start(fd);
}



SYS_PUBLIC sys_error_t sys_fd_iter_get(sys_fd_iterator_t iterator,char* name,u32 size){
	return _sys_syscall_fd_iter_get(iterator,name,size);
}



SYS_PUBLIC sys_fd_iterator_t sys_fd_iter_next(sys_fd_iterator_t iterator){
	return _sys_syscall_fd_iter_next(iterator);
}



SYS_PUBLIC sys_error_t sys_fd_iter_stop(sys_fd_iterator_t iterator){
	return _sys_syscall_fd_iter_stop(iterator);
}
