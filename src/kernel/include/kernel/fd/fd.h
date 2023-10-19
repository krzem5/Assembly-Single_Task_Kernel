#ifndef _KERNEL_FD_FD_H_
#define _KERNEL_FD_FD_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <kernel/vfs2/name.h>
#include <kernel/vfs2/node.h>



#define _ERROR_INVALID_FLAGS -1
#define _ERROR_INVALID_FD -2
#define _ERROR_INVALID_POINTER -3
#define _ERROR_OUT_OF_FDS -4
#define _ERROR_NOT_FOUND -5
#define _ERROR_UNSUPPORTED_OPERATION -6
#define _ERROR_NO_RELATIVE -7
#define _ERROR_NOT_EMPTY -8
#define _ERROR_DIFFERENT_FS -9
#define _ERROR_DIFFERENT_TYPE -10
#define _ERROR_NO_SPACE -11

#define _FLAG_READ 1
#define _FLAG_WRITE 2
#define _FLAG_APPEND 4
#define _FLAG_CREATE 8
#define _FLAG_DIRECTORY 16

#define _SEEK_SET 0
#define _SEEK_ADD 1
#define _SEEK_END 2



typedef struct _{
	handle_t handle;
	lock_t lock;
	vfs2_node_t* node;
	u64 offset;
	u8 flags;
} _t;



typedef struct __STAT{
	u8 type;
	u8 vfs_index;
	u8 name_length;
	char name[64];
	u64 size;
} _stat_t;



typedef struct __ITERATOR{
	handle_t handle;
	lock_t lock;
	vfs2_node_t* node;
	u64 pointer;
	vfs2_name_t* current_name;
} _iterator_t;



s64 _open(handle_id_t root,const char* path,u32 length,u32 flags);



s64 _close(handle_id_t fd);



s64 _read(handle_id_t fd,void* buffer,u64 count);



s64 _write(handle_id_t fd,const void* buffer,u64 count);



s64 _seek(handle_id_t fd,u64 offset,u32 type);



s64 _resize(handle_id_t fd,u64 size,u32 flags);



s64 _stat(handle_id_t fd,_stat_t* out);



s64 _dup(handle_id_t fd,u32 flags);



s64 _path(handle_id_t fd,char* buffer,u32 buffer_length);



s64 _iter_start(handle_id_t fd);



s64 _iter_get(handle_id_t iterator,char* buffer,u32 buffer_length);



s64 _iter_next(handle_id_t iterator);



s64 _iter_stop(handle_id_t iterator);



#endif
