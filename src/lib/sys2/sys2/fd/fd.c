#include <sys2/error/error.h>
#include <sys2/fd/fd.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_fd_t sys2_fd_open(sys2_fd_t fd,const char* path,u32 flags){
	return _sys2_syscall_fd_open(fd,path,flags);
}



SYS2_PUBLIC sys2_error_t sys2_fd_close(sys2_fd_t fd){
	return _sys2_syscall_fd_close(fd);
}



SYS2_PUBLIC u64 sys2_fd_read(sys2_fd_t fd,void* buffer,u64 size,u32 flags){
	return _sys2_syscall_fd_read(fd,buffer,size,flags);
}



SYS2_PUBLIC u64 sys2_fd_write(sys2_fd_t fd,const void* buffer,u64 size,u32 flags){
	return _sys2_syscall_fd_write(fd,buffer,size,flags);
}



SYS2_PUBLIC u64 sys2_fd_seek(sys2_fd_t fd,u64 offset,u32 type){
	return _sys2_syscall_fd_seek(fd,offset,type);
}



SYS2_PUBLIC u64 sys2_fd_resize(sys2_fd_t fd,u64 size,u32 flags){
	return _sys2_syscall_fd_resize(fd,size,flags);
}



SYS2_PUBLIC sys2_error_t sys2_fd_stat(sys2_fd_t fd,sys2_fd_stat_t* out){
	return _sys2_syscall_fd_stat(fd,out,sizeof(sys2_fd_stat_t));
}



SYS2_PUBLIC sys2_fd_t sys2_fd_dup(sys2_fd_t fd,u32 flags){
	return _sys2_syscall_fd_dup(fd,flags);
}



SYS2_PUBLIC sys2_error_t sys2_fd_path(sys2_fd_t fd,char* path,u32 size){
	return _sys2_syscall_fd_path(fd,path,size);
}



SYS2_PUBLIC sys2_fd_iterator_t sys2_fd_iter_start(sys2_fd_t fd){
	return _sys2_syscall_fd_iter_start(fd);
}



SYS2_PUBLIC sys2_error_t sys2_fd_iter_get(sys2_fd_iterator_t iterator,char* name,u32 size){
	return _sys2_syscall_fd_iter_get(iterator,name,size);
}



SYS2_PUBLIC sys2_fd_iterator_t sys2_fd_iter_next(sys2_fd_iterator_t iterator){
	return _sys2_syscall_fd_iter_next(iterator);
}



SYS2_PUBLIC sys2_error_t sys2_fd_iter_stop(sys2_fd_iterator_t iterator){
	return _sys2_syscall_fd_iter_stop(iterator);
}
