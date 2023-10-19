#ifndef _KERNEL_FD2_FD2_H_
#define _KERNEL_FD2_FD2_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <kernel/vfs2/node.h>



#define FD2_ERROR_INVALID_FLAGS -1
#define FD2_ERROR_INVALID_FD -2
#define FD2_ERROR_INVALID_POINTER -3
#define FD2_ERROR_OUT_OF_FDS -4
#define FD2_ERROR_NOT_FOUND -5
#define FD2_ERROR_UNSUPPORTED_OPERATION -6
#define FD2_ERROR_NO_RELATIVE -7
#define FD2_ERROR_NOT_EMPTY -8
#define FD2_ERROR_DIFFERENT_FS -9
#define FD2_ERROR_DIFFERENT_TYPE -10
#define FD2_ERROR_NO_SPACE -11

#define FD2_FLAG_READ 1
#define FD2_FLAG_WRITE 2
#define FD2_FLAG_APPEND 4
#define FD2_FLAG_CREATE 8
#define FD2_FLAG_DIRECTORY 16

#define FD2_SEEK_SET 0
#define FD2_SEEK_ADD 1
#define FD2_SEEK_END 2



typedef struct _FD2{
	handle_t handle;
	lock_t lock;
	vfs2_node_t* node;
	u64 offset;
	u8 flags;
} fd2_t;



typedef struct _FD2_STAT{
	u8 type;
	u8 vfs_index;
	u8 name_length;
	char name[64];
	u64 size;
} fd2_stat_t;



s64 fd2_open(handle_id_t root,const char* path,u32 length,u32 flags);



s64 fd2_close(handle_id_t fd);



s64 fd2_read(handle_id_t fd,void* buffer,u64 count);



s64 fd2_write(handle_id_t fd,const void* buffer,u64 count);



s64 fd2_seek(handle_id_t fd,u64 offset,u32 type);



s64 fd2_resize(handle_id_t fd,u64 size,u32 flags);



s64 fd2_stat(handle_id_t fd,fd2_stat_t* out);



s64 fd2_dup(handle_id_t fd,u32 flags);



s64 fd2_path(handle_id_t fd,char* buffer,u32 buffer_length);



s64 fd2_iter_start(handle_id_t fd);



s64 fd2_iter_get(handle_id_t iterator,char* buffer,u32 buffer_length);



s64 fd2_iter_next(handle_id_t iterator);



s64 fd2_iter_stop(handle_id_t iterator);



#endif
