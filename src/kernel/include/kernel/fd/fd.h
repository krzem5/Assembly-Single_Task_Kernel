#ifndef _KERNEL_FD_FD_H_
#define _KERNEL_FD_FD_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <kernel/vfs/allocator.h>



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

#define FD_SEEK_SET 0
#define FD_SEEK_ADD 1
#define FD_SEEK_END 2

#define FD_RELATIVE_PARENT 0
#define FD_RELATIVE_PREV_SIBLING 1
#define FD_RELATIVE_NEXT_SIBLING 2
#define FD_RELATIVE_FIRST_CHILD 3



typedef struct _FD_DATA{
	handle_t handle;
	lock_t lock;
	vfs_node_id_t node_id;
	u64 offset;
	u8 flags;
} fd_data_t;



typedef struct _FD_STAT{
	vfs_node_id_t node_id;
	u8 type;
	u8 vfs_index;
	u8 name_length;
	char name[64];
	u64 size;
} fd_stat_t;



void fd_clear(void);



s64 fd_open(handle_id_t root,const char* path,u32 length,u8 flags);



s64 fd_close(handle_id_t fd);



s64 fd_delete(handle_id_t fd);



s64 fd_read(handle_id_t fd,void* buffer,u64 count);



s64 fd_write(handle_id_t fd,const void* buffer,u64 count);



s64 fd_seek(handle_id_t fd,u64 offset,u8 type);



s64 fd_resize(handle_id_t fd,u64 size);



s64 fd_absolute_path(handle_id_t fd,char* buffer,u32 buffer_length);



s64 fd_stat(handle_id_t fd,fd_stat_t* out);



s64 fd_get_relative(handle_id_t fd,u8 relative,u8 flags);



s64 fd_move(handle_id_t fd,handle_id_t dst_fd);



#endif
