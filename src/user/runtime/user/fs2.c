#include <user/fs2.h>
#include <user/syscall.h>
#include <user/types.h>



int fs2_open(int fd,const char* path,u8 flags){
	u32 length=0;
	while (path[length]){
		length++;
	}
	return _syscall_fd_open(fd,path,length,flags);
}



int fs2_close(int fd){
	return _syscall_fd_close(fd);
}



int fs2_delete(int fd){
	return _syscall_fd_delete(fd);
}



s64 fs2_read(int fd,void* buffer,u64 count){
	return _syscall_fd_read(fd,buffer,count);
}



s64 fs2_write(int fd,const void* buffer,u64 count){
	return _syscall_fd_write(fd,buffer,count);
}



s64 fs2_seek(int fd,u64 offset,u8 type){
	return _syscall_fd_seek(fd,offset,type);
}



int fs2_resize(int fd,u64 size){
	return _syscall_fd_resize(fd,size);
}



int fs2_absolute_path(int fd,char* buffer,u32 buffer_length){
	return _syscall_fd_absolute_path(fd,buffer,buffer_length);
}



int fs2_stat(int fd,fs2_stat_t* stat){
	return _syscall_fd_stat(fd,stat,sizeof(fs2_stat_t));
}



int fs2_get_relative(int fd,u8 relative,u8 flags){
	return _syscall_fd_get_relative(fd,relative,flags);
}



int fs2_move(int fd,int dst_fd){
	return _syscall_fd_move(fd,dst_fd);
}
