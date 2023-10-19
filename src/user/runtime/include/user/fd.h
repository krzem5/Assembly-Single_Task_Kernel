#ifndef _USER_FD_H_
#define _USER_FD_H_ 1
#include <user/types.h>



#define FS2_ERROR_INVALID_FLAGS -1
#define FS2_ERROR_INVALID_FD -2
#define FS2_ERROR_INVALID_POINTER -3
#define FS2_ERROR_OUT_OF_FDS -4
#define FS2_ERROR_NOT_FOUND -5
#define FS2_ERROR_UNSUPPORTED_OPERATION -6
#define FS2_ERROR_NO_RELATIVE -7
#define FS2_ERROR_NOT_EMPTY -8
#define FS2_ERROR_DIFFERENT_FS -9
#define FS2_ERROR_DIFFERENT_TYPE -10
#define FS2_ERROR_NO_SPACE -11

#define FS2_FLAG_READ 1
#define FS2_FLAG_WRITE 2
#define FS2_FLAG_APPEND 4
#define FS2_FLAG_CREATE 8
#define FS2_FLAG_DIRECTORY 16
#define FS2_FLAG_DELETE_AT_EXIT 32

#define FS2_SEEK_SET 0
#define FS2_SEEK_ADD 1
#define FS2_SEEK_END 2

#define FS2_STAT_TYPE_FILE 1
#define FS2_STAT_TYPE_DIRECTORY 2



typedef struct _FD_STAT{
	u8 type;
	u8 fs2_index;
	u8 name_length;
	char name[64];
	u64 size;
} fd_stat_t;



s64 fd_open(u64 fd,const char* path,u32 length,u32 flags);



s64 fd_close(u64 fd);



s64 fd_read(u64 fd,void* buffer,u32 size);



s64 fd_write(u64 fd,const void* buffer,u32 size);



s64 fd_seek(u64 fd,u64 offset,u32 type);



s64 fd_resize(u64 fd,u64 size,u32 flags);



s64 fd_stat(u64 fd,fd_stat_t* out);



s64 fd_dup(u64 fd,u32 flags);



s64 fd_path(u64 fd,char* buffer,u32 size);



s64 fd_iter_start(u64 fd);



s64 fd_iter_get(u64 iterator,char* buffer,u32 size);



s64 fd_iter_next(u64 iterator);



s64 fd_iter_stop(u64 iterator);



#endif
