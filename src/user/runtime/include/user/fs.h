#ifndef _USER_FS_H_
#define _USER_FS_H_ 1
#include <user/types.h>



#define FS_ERROR_INVALID_FLAGS -1
#define FS_ERROR_INVALID_FD -2
#define FS_ERROR_INVALID_POINTER -3
#define FS_ERROR_OUT_OF_FDS -4
#define FS_ERROR_NOT_FOUND -5
#define FS_ERROR_UNSUPPORTED_OPERATION -6
#define FS_ERROR_NO_RELATIVE -7
#define FS_ERROR_NOT_EMPTY -8

#define FS_FLAG_READ 1
#define FS_FLAG_WRITE 2
#define FS_FLAG_APPEND 4
#define FS_FLAG_CREATE 8
#define FS_FLAG_DIRECTORY 16

#define FS_SEEK_SET 0
#define FS_SEEK_ADD 1
#define FS_SEEK_END 2

#define FS_RELATIVE_PARENT 0
#define FS_RELATIVE_PREV_SIBLING 1
#define FS_RELATIVE_NEXT_SIBLING 2
#define FS_RELATIVE_FIRST_CHILD 3

#define FS_STAT_TYPE_FILE 1
#define FS_STAT_TYPE_DIRECTORY 2



typedef struct _FS_STAT{
	u64 node_id;
	u8 type;
	u8 fs_index;
	u8 name_length;
	char name[64];
	u64 size;
} fs_stat_t;



int fs_open(int fd,const char* path,u8 flags);



int fs_close(int fd);



int fs_delete(int fd);



s64 fs_read(int fd,void* buffer,u64 count);



s64 fs_write(int fd,const void* buffer,u64 count);



s64 fs_seek(int fd,u64 offset,u8 type);



int fs_absolute_path(int fd,char* buffer,u32 buffer_length);



int fs_stat(int fd,fs_stat_t* stat);



int fs_get_relative(int fd,u8 relative,u8 flags);



int fs_dup(int fd,u8 flags);



#endif
