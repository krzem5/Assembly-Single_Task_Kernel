#include <user/fd.h>
#include <user/syscall.h>
#include <user/types.h>



s64 fd_open(u64 fd,const char* path,u32 flags){
	u64 length=0;
	for (;path[length];length++);
	return _syscall_fd_open(fd,path,length,flags);
}



s64 fd_close(u64 fd){
	return _syscall_fd_close(fd);
}



s64 fd_read(u64 fd,void* buffer,u32 size){
	return _syscall_fd_read(fd,buffer,size);
}



s64 fd_write(u64 fd,const void* buffer,u32 size){
	return _syscall_fd_write(fd,buffer,size);
}



s64 fd_seek(u64 fd,u64 offset,u32 type){
	return _syscall_fd_seek(fd,offset,type);
}



s64 fd_resize(u64 fd,u64 size,u32 flags){
	return _syscall_fd_resize(fd,size,flags);
}



s64 fd_stat(u64 fd,fd_stat_t* out){
	return _syscall_fd_stat(fd,out,sizeof(fd_stat_t));
}



s64 fd_dup(u64 fd,u32 flags){
	return _syscall_fd_dup(fd,flags);
}



s64 fd_path(u64 fd,char* buffer,u32 size){
	return _syscall_fd_path(fd,buffer,size);
}



s64 fd_iter_start(u64 fd){
	return _syscall_fd_iter_start(fd);
}



s64 fd_iter_get(u64 iterator,char* buffer,u32 size){
	return _syscall_fd_iter_get(iterator,buffer,size);
}



s64 fd_iter_next(u64 iterator){
	return _syscall_fd_iter_next(iterator);
}



s64 fd_iter_stop(u64 iterator){
	return _syscall_fd_iter_stop(iterator);
}
