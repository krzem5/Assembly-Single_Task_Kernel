#include <sys/fd.h>
#include <sys/syscall.h>
#include <sys/types.h>



SYS_PUBLIC s64 sys_fd_open(u64 fd,const char* path,u32 flags){
	return _syscall_fd_open(fd,path,flags);
}



SYS_PUBLIC s64 sys_fd_close(u64 fd){
	return _syscall_fd_close(fd);
}



SYS_PUBLIC s64 sys_fd_read(u64 fd,void* buffer,u64 size,u32 flags){
	return _syscall_fd_read(fd,buffer,size,flags);
}



SYS_PUBLIC s64 sys_fd_write(u64 fd,const void* buffer,u64 size,u32 flags){
	return _syscall_fd_write(fd,buffer,size,flags);
}



SYS_PUBLIC s64 sys_fd_seek(u64 fd,u64 offset,u32 type){
	return _syscall_fd_seek(fd,offset,type);
}



SYS_PUBLIC s64 sys_fd_resize(u64 fd,u64 size,u32 flags){
	return _syscall_fd_resize(fd,size,flags);
}



SYS_PUBLIC s64 sys_fd_stat(u64 fd,sys_fd_stat_t* out){
	return _syscall_fd_stat(fd,out,sizeof(sys_fd_stat_t));
}



SYS_PUBLIC s64 sys_fd_dup(u64 fd,u32 flags){
	return _syscall_fd_dup(fd,flags);
}



SYS_PUBLIC s64 sys_fd_path(u64 fd,char* buffer,u32 size){
	return _syscall_fd_path(fd,buffer,size);
}



SYS_PUBLIC s64 sys_fd_iter_start(u64 fd){
	return _syscall_fd_iter_start(fd);
}



SYS_PUBLIC s64 sys_fd_iter_get(u64 iterator,char* buffer,u32 size){
	return _syscall_fd_iter_get(iterator,buffer,size);
}



SYS_PUBLIC s64 sys_fd_iter_next(u64 iterator){
	return _syscall_fd_iter_next(iterator);
}



SYS_PUBLIC s64 sys_fd_iter_stop(u64 iterator){
	return _syscall_fd_iter_stop(iterator);
}
