#ifndef _KERNEL_FD_FD_H_
#define _KERNEL_FD_FD_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <kernel/memory/smm.h>
#include <kernel/vfs/node.h>



#define FD_ERROR_INVALID_FLAGS -1
#define FD_ERROR_INVALID_FD -2
#define FD_ERROR_INVALID_POINTER -3
#define FD_ERROR_OUT_OF_FDS -4
#define FD_ERROR_NOT_FOUND -5
#define FD_ERROR_UNSUPPORTED_OPERATION -6
#define FD_ERROR_NO_RELATIVE -7
#define FD_ERROR_NOT_EMPTY -8
#define FD_ERROR_DIFFERENT_FS -9
#define FD_ERROR_DIFFERENT_TYPE -10
#define FD_ERROR_NO_SPACE -11

#define FD_FLAG_READ 1
#define FD_FLAG_WRITE 2
#define FD_FLAG_APPEND 4
#define FD_FLAG_CREATE 8
#define FD_FLAG_DIRECTORY 16
#define FD_FLAG_IGNORE_LINKS 32
#define FD_FLAG_NONBLOCKING 64

#define FD_SEEK_SET 0
#define FD_SEEK_ADD 1
#define FD_SEEK_END 2



typedef struct _FD{
	handle_t handle;
	spinlock_t lock;
	vfs_node_t* node;
	u64 offset;
	u8 flags;
} fd_t;



typedef struct _FD_STAT{
	u8 type;
	u8 name_length;
	u64 fs_handle;
	u64 size;
	char name[256];
} fd_stat_t;



typedef struct _FD_ITERATOR{
	handle_t handle;
	spinlock_t lock;
	vfs_node_t* node;
	u64 pointer;
	string_t* current_name;
} fd_iterator_t;



s64 fd_open(handle_id_t root,const char* path,u32 length,u32 flags);



s64 fd_close(handle_id_t fd);



s64 fd_read(handle_id_t fd,void* buffer,u64 count,u32 flags);



s64 fd_write(handle_id_t fd,const void* buffer,u64 count,u32 flags);



s64 fd_seek(handle_id_t fd,u64 offset,u32 type);



s64 fd_resize(handle_id_t fd,u64 size,u32 flags);



s64 fd_stat(handle_id_t fd,fd_stat_t* out);



s64 fd_dup(handle_id_t fd,u32 flags);



s64 fd_path(handle_id_t fd,char* buffer,u32 buffer_length);



s64 fd_iter_start(handle_id_t fd);



s64 fd_iter_get(handle_id_t iterator,char* buffer,u32 buffer_length);



s64 fd_iter_next(handle_id_t iterator);



s64 fd_iter_stop(handle_id_t iterator);



#endif
