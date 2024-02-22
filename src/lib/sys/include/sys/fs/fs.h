#ifndef _SYS_FS_FS_H_
#define _SYS_FS_FS_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_fs_t;



typedef struct _SYS_FS_DATA{
	char type[64];
	u64 partition;
	u8 guid[16];
	char mount_path[256];
} sys_fs_data_t;



sys_fs_t sys_fs_iter_start(void);



sys_fs_t sys_fs_iter_next(sys_fs_t fs);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_fs_get_data(sys_fs_t fs,sys_fs_data_t* out);



sys_error_t __attribute__((access(read_only,2),nonnull)) sys_fs_mount(sys_fs_t fs,const char* path);



#endif
