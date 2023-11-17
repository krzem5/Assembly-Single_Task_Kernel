#ifndef _SYS_FD_H_
#define _SYS_FD_H_ 1
#include <sys/types.h>



#define SYS_FD_ERROR_INVALID_FLAGS -1
#define SYS_FD_ERROR_INVALID_FD -2
#define SYS_FD_ERROR_INVALID_POINTER -3
#define SYS_FD_ERROR_OUT_OF_FDS -4
#define SYS_FD_ERROR_NOT_FOUND -5
#define SYS_FD_ERROR_UNSUPPORTED_OPERATION -6
#define SYS_FD_ERROR_NO_RELATIVE -7
#define SYS_FD_ERROR_NOT_EMPTY -8
#define SYS_FD_ERROR_DIFFERENT_FS -9
#define SYS_FD_ERROR_DIFFERENT_TYPE -10
#define SYS_FD_ERROR_NO_SPACE -11

#define SYS_FD_FLAG_READ 1
#define SYS_FD_FLAG_WRITE 2
#define SYS_FD_FLAG_APPEND 4
#define SYS_FD_FLAG_CREATE 8
#define SYS_FD_FLAG_DIRECTORY 16
#define SYS_FD_FLAG_IGNORE_LINKS 32
#define SYS_FD_FLAG_NONBLOCKING 64
#define SYS_FD_FLAG_PIPE_PEEK 128
#define SYS_FD_FLAG_DELETE_ON_EXIT 256

#define SYS_FD_SEEK_SET 0
#define SYS_FD_SEEK_ADD 1
#define SYS_FD_SEEK_END 2

#define SYS_FD_PERMISSION_ROOT_READ 256
#define SYS_FD_PERMISSION_ROOT_WRITE 128
#define SYS_FD_PERMISSION_ROOT_EXEC 64
#define SYS_FD_PERMISSION_USER_READ 32
#define SYS_FD_PERMISSION_USER_WRITE 16
#define SYS_FD_PERMISSION_USER_EXEC 8
#define SYS_FD_PERMISSION_OTHER_READ 4
#define SYS_FD_PERMISSION_OTHER_WRITE 2
#define SYS_FD_PERMISSION_OTHER_EXEC 1

#define SYS_FD_STAT_TYPE_FILE 1
#define SYS_FD_STAT_TYPE_DIRECTORY 2
#define SYS_FD_STAT_TYPE_LINK 3
#define SYS_FD_STAT_TYPE_PIPE 4

#define SYS_FD_STAT_FLAG_VIRTUAL 1



typedef struct _SYS_FD_STAT{
	u8 type;
	u8 flags;
	u16 permissions;
	u8 name_length;
	u64 fs_handle;
	u64 size;
	u64 time_access;
	u64 time_modify;
	u64 time_change;
	u64 time_birth;
	u32 gid;
	u32 uid;
	char name[256];
} sys_fd_stat_t;



s64 sys_fd_open(u64 fd,const char* path,u32 flags);



s64 sys_fd_close(u64 fd);



s64 sys_fd_read(u64 fd,void* buffer,u64 size,u32 flags);



s64 sys_fd_write(u64 fd,const void* buffer,u64 size,u32 flags);



s64 sys_fd_seek(u64 fd,u64 offset,u32 type);



s64 sys_fd_resize(u64 fd,u64 size,u32 flags);



s64 sys_fd_stat(u64 fd,sys_fd_stat_t* out);



s64 sys_fd_dup(u64 fd,u32 flags);



s64 sys_fd_path(u64 fd,char* buffer,u32 size);



s64 sys_fd_iter_start(u64 fd);



s64 sys_fd_iter_get(u64 iterator,char* buffer,u32 size);



s64 sys_fd_iter_next(u64 iterator);



s64 sys_fd_iter_stop(u64 iterator);



#endif
