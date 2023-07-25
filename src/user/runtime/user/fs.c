#include <user/fs.h>
#include <user/syscall.h>
#include <user/types.h>



int fs_open(const char* path,u8 flags){
	u32 length=0;
	while (path[length]){
		length++;
	}
	return _syscall_fd_open(path,length,flags);
}



int fs_close(int fd){
	return _syscall_fd_close(fd);
}



int fs_delete(int fd){
	return _syscall_fd_delete(fd);
}



s64 fs_read(int fd,void* buffer,u64 count){
	return _syscall_fd_read(fd,buffer,count);
}



s64 fs_write(int fd,const void* buffer,u64 count){
	return _syscall_fd_write(fd,buffer,count);
}



s64 fs_seek(int fd,u64 offset,u8 type){
	return _syscall_fd_seek(fd,offset,type);
}



int fs_absolute_path(int fd,char* buffer,u32 buffer_length){
	return 0;
}



int fs_stat(int fd,fs_stat_t* stat){
	return _syscall_fd_stat(fd,stat,sizeof(fs_stat_t));
}



int fs_get_relative(int fd,u8 relative){
	return _syscall_fd_get_relative(fd,relative);
}



int fs_dup(int fd,u8 flags){
	return _syscall_fd_dup(fd,flags);
}
