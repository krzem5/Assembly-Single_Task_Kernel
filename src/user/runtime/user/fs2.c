#include <user/fs2.h>
#include <user/syscall.h>
#include <user/types.h>



s64 fs2_open(u64 fd,const char* path,u8 flags){
	u32 length=0;
	while (path[length]){
		length++;
	}
	return _syscall_fd2_open(fd,path,length,flags);
}



s64 fs2_close(u64 fd){
	return _syscall_fd2_close(fd);
}



s64 fs2_delete(u64 fd){
	return _syscall_fd2_delete(fd);
}



s64 fs2_read(u64 fd,void* buffer,u64 count){
	return _syscall_fd2_read(fd,buffer,count);
}



s64 fs2_write(u64 fd,const void* buffer,u64 count){
	return _syscall_fd2_write(fd,buffer,count);
}



s64 fs2_seek(u64 fd,u64 offset,u8 type){
	return _syscall_fd2_seek(fd,offset,type);
}



s64 fs2_resize(u64 fd,u64 size){
	return _syscall_fd2_resize(fd,size);
}



s64 fs2_absolute_path(u64 fd,char* buffer,u32 buffer_length){
	return _syscall_fd2_absolute_path(fd,buffer,buffer_length);
}



s64 fs2_stat(u64 fd,fs2_stat_t* stat){
	return _syscall_fd2_stat(fd,stat,sizeof(fs2_stat_t));
}



s64 fs2_move(u64 fd,u64 dst_fd){
	return _syscall_fd2_move(fd,dst_fd);
}
