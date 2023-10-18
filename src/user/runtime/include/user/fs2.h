#ifndef _USER_FS2_H_
#define _USER_FS2_H_ 1
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

#define FS2_RELATIVE_PARENT 0
#define FS2_RELATIVE_PREV_SIBLING 1
#define FS2_RELATIVE_NEXT_SIBLING 2
#define FS2_RELATIVE_FIRST_CHILD 3

#define FS2_STAT_TYPE_FILE 1
#define FS2_STAT_TYPE_DIRECTORY 2



typedef struct _FS2_STAT{
	u8 type;
	u8 fs2_index;
	u8 name_length;
	char name[64];
	u64 size;
} fs2_stat_t;



int fs2_open(int fd,const char* path,u8 flags);



int fs2_close(int fd);



int fs2_delete(int fd);



s64 fs2_read(int fd,void* buffer,u64 count);



s64 fs2_write(int fd,const void* buffer,u64 count);



s64 fs2_seek(int fd,u64 offset,u8 type);



int fs2_resize(int fd,u64 size);



int fs2_absolute_path(int fd,char* buffer,u32 buffer_length);



int fs2_stat(int fd,fs2_stat_t* stat);



int fs2_get_relative(int fd,u8 relative,u8 flags);



int fs2_move(int fd,int dst_fd);



#endif
