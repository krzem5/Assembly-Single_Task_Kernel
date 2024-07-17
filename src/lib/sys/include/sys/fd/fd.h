#ifndef _SYS_FD_FD_H_
#define _SYS_FD_FD_H_ 1
#include <sys/error/error.h>
#include <sys/handle/handle.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/types.h>



#define SYS_FD_FLAG_READ 1
#define SYS_FD_FLAG_WRITE 2
#define SYS_FD_FLAG_APPEND 4
#define SYS_FD_FLAG_CREATE 8
#define SYS_FD_FLAG_DIRECTORY 16
#define SYS_FD_FLAG_IGNORE_LINKS 32
#define SYS_FD_FLAG_NONBLOCKING 64
#define SYS_FD_FLAG_PIPE_PEEK 128
#define SYS_FD_FLAG_DELETE_ON_EXIT 256
#define SYS_FD_FLAG_EXCLUSIVE_CREATE 512
#define SYS_FD_FLAG_LINK 1024
#define SYS_FD_FLAG_CLOSE_PIPE 2048
#define SYS_FD_FLAG_FIND_LINKS 4096

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
#define SYS_FD_STAT_TYPE_SOCKET 5

#define SYS_FD_STAT_FLAG_VIRTUAL 1

#define SYS_FD_DUP_CWD 0x7ffffffffffffffcull
#define SYS_FD_DUP_STDIN 0x7ffffffffffffffdull
#define SYS_FD_DUP_STDOUT 0x7ffffffffffffffeull
#define SYS_FD_DUP_STDERR 0x7fffffffffffffffull

#define SYS_FD_ACL_FLAG_STAT 1
#define SYS_FD_ACL_FLAG_DUP 2
#define SYS_FD_ACL_FLAG_IO 4
#define SYS_FD_ACL_FLAG_CLOSE 8

#define SYS_FD_ITERATOR_ACL_FLAG_ACCESS 1



typedef u64 sys_fd_t;



typedef u64 sys_fd_iterator_t;



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
	sys_gid_t gid;
	sys_uid_t uid;
	sys_handle_t lock_handle;
	char name[256];
} sys_fd_stat_t;



sys_fd_t __attribute__((access(read_only,2),nonnull,warn_unused_result)) sys_fd_open(sys_fd_t fd,const char* path,u32 flags);



sys_error_t sys_fd_close(sys_fd_t fd);



u64 __attribute__((access(write_only,2,3),nonnull,warn_unused_result)) sys_fd_read(sys_fd_t fd,void* buffer,u64 size,u32 flags);



u64 __attribute__((access(read_only,2,3),nonnull,warn_unused_result)) sys_fd_write(sys_fd_t fd,const void* buffer,u64 size,u32 flags);



u64 sys_fd_seek(sys_fd_t fd,u64 offset,u32 type);



u64 __attribute__((warn_unused_result)) sys_fd_resize(sys_fd_t fd,u64 size,u32 flags);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_fd_stat(sys_fd_t fd,sys_fd_stat_t* out);



sys_fd_t sys_fd_dup(sys_fd_t fd,u32 flags);



sys_error_t sys_fd_link(sys_fd_t parent,sys_fd_t fd);



sys_error_t sys_fd_unlink(sys_fd_t fd);



sys_error_t __attribute__((access(write_only,2,3),nonnull)) sys_fd_path(sys_fd_t fd,char* path,u32 size);



sys_error_t __attribute__((access(read_only,2,3),nonnull)) sys_fd_stream(sys_fd_t src_fd,const sys_fd_t* dst_fds,u32 dst_fd_count,u64 length);



sys_error_t sys_fd_lock(sys_fd_t fd,sys_handle_t handle);



sys_error_t sys_fd_unlock(sys_fd_t fd);



sys_error_t sys_fd_get_event(sys_fd_t fd,bool is_write_event);



sys_fd_iterator_t sys_fd_iter_start(sys_fd_t fd);



sys_error_t __attribute__((access(write_only,2,3),nonnull)) sys_fd_iter_get(sys_fd_iterator_t iterator,char* name,u32 size);



sys_fd_iterator_t sys_fd_iter_next(sys_fd_iterator_t iterator);



sys_error_t sys_fd_iter_stop(sys_fd_iterator_t iterator);



#endif
