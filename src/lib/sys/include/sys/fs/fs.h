#ifndef _SYS_FS_FS_H_
#define _SYS_FS_FS_H_ 1
#include <sys/error/error.h>
#include <sys/partition/partition.h>
#include <sys/types.h>



#define SYS_FS_DESCRIPTOR_FLAG_CAN_FORMAT 1



typedef u64 sys_fs_t;



typedef u64 sys_fs_descriptor_t;



typedef struct _SYS_FS_DATA{
	char type[64];
	u64 partition;
	u8 guid[16];
	u8 uuid[16];
	char mount_path[256];
} sys_fs_data_t;



typedef struct _SYS_FS_DESCRIPTOR_DATA{
	char name[64];
	u32 flags;
} sys_fs_descriptor_data_t;



sys_fs_t sys_fs_iter_start(void);



sys_fs_t sys_fs_iter_next(sys_fs_t fs);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_fs_get_data(sys_fs_t fs,sys_fs_data_t* out);



sys_error_t __attribute__((access(read_only,2),nonnull)) sys_fs_mount(sys_fs_t fs,const char* path);



sys_error_t sys_fs_format(sys_partition_t partition,sys_fs_descriptor_t fs_descriptor);



sys_fs_descriptor_t sys_fs_descriptor_iter_start(void);



sys_fs_descriptor_t sys_fs_descriptor_iter_next(sys_fs_descriptor_t fs_descriptor);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_fs_descriptor_get_data(sys_fs_descriptor_t fs_descriptor,sys_fs_descriptor_data_t* out);



#endif
