#ifndef _SYS2_FD_FD_H_
#define _SYS2_FD_FD_H_ 1
#include <sys2/error/error.h>
#include <sys2/id/group.h>
#include <sys2/id/user.h>
#include <sys2/types.h>



#define SYS2_FD_FLAG_READ 1
#define SYS2_FD_FLAG_WRITE 2
#define SYS2_FD_FLAG_APPEND 4
#define SYS2_FD_FLAG_CREATE 8
#define SYS2_FD_FLAG_DIRECTORY 16
#define SYS2_FD_FLAG_IGNORE_LINKS 32
#define SYS2_FD_FLAG_NONBLOCKING 64
#define SYS2_FD_FLAG_PIPE_PEEK 128
#define SYS2_FD_FLAG_DELETE_ON_EXIT 256

#define SYS2_FD_SEEK_SET 0
#define SYS2_FD_SEEK_ADD 1
#define SYS2_FD_SEEK_END 2

#define SYS2_FD_PERMISSION_ROOT_READ 256
#define SYS2_FD_PERMISSION_ROOT_WRITE 128
#define SYS2_FD_PERMISSION_ROOT_EXEC 64
#define SYS2_FD_PERMISSION_USER_READ 32
#define SYS2_FD_PERMISSION_USER_WRITE 16
#define SYS2_FD_PERMISSION_USER_EXEC 8
#define SYS2_FD_PERMISSION_OTHER_READ 4
#define SYS2_FD_PERMISSION_OTHER_WRITE 2
#define SYS2_FD_PERMISSION_OTHER_EXEC 1

#define SYS2_FD_STAT_TYPE_FILE 1
#define SYS2_FD_STAT_TYPE_DIRECTORY 2
#define SYS2_FD_STAT_TYPE_LINK 3
#define SYS2_FD_STAT_TYPE_PIPE 4
#define SYS2_FD_STAT_TYPE_SOCKET 5

#define SYS2_FD_STAT_FLAG_VIRTUAL 1



typedef u64 sys2_fd_t;



typedef u64 sys2_fd_iterator_t;



typedef struct _SYS2_FD_STAT{
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
	sys2_gid_t gid;
	sys2_uid_t uid;
	char name[256];
} sys2_fd_stat_t;



sys2_fd_t sys2_fd_open(sys2_fd_t fd,const char* path,u32 flags);



sys2_error_t sys2_fd_close(sys2_fd_t fd);



u64 sys2_fd_read(sys2_fd_t fd,void* buffer,u64 size,u32 flags);



u64 sys2_fd_write(sys2_fd_t fd,const void* buffer,u64 size,u32 flags);



u64 sys2_fd_seek(sys2_fd_t fd,u64 offset,u32 type);



u64 sys2_fd_resize(sys2_fd_t fd,u64 size,u32 flags);



sys2_error_t sys2_fd_stat(sys2_fd_t fd,sys2_fd_stat_t* out);



sys2_fd_t sys2_fd_dup(sys2_fd_t fd,u32 flags);



sys2_error_t sys2_fd_path(sys2_fd_t fd,char* path,u32 size);



sys2_fd_iterator_t sys2_fd_iter_start(sys2_fd_t fd);



sys2_error_t sys2_fd_iter_get(sys2_fd_iterator_t iterator,char* name,u32 size);



sys2_fd_iterator_t sys2_fd_iter_next(sys2_fd_iterator_t iterator);



sys2_error_t sys2_fd_iter_stop(sys2_fd_iterator_t iterator);



#endif
