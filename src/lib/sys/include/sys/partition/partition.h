#ifndef _SYS_PARTITION_PARTITION_H_
#define _SYS_PARTITION_PARTITION_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_partition_t;



typedef struct _SYS_PARTITION_DATA{
	char name[64];
	char type[64];
	u64 drive;
	u32 index;
	u64 start_lba;
	u64 end_lba;
	u64 fs;
} sys_partition_data_t;



sys_partition_t sys_partition_iter_start(void);



sys_partition_t sys_partition_iter_next(sys_partition_t partition);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_partition_get_data(sys_partition_t partition,sys_partition_data_t* out);



#endif
