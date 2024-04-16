#ifndef _SYS_PARTITION_PARTITION_H_
#define _SYS_PARTITION_PARTITION_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



#define SYS_PARTITION_TABLE_DESCRIPTOR_FLAG_CAN_FORMAT 1



typedef u64 sys_partition_t;



typedef u64 sys_partition_table_descriptor_t;



typedef struct _SYS_PARTITION_DATA{
	char name[64];
	char type[64];
	u64 drive;
	u32 index;
	u64 start_lba;
	u64 end_lba;
	u64 fs;
} sys_partition_data_t;



typedef struct _SYS_PARTITION_TABLE_DESCRIPTOR_DATA{
	char name[64];
	u32 flags;
} sys_partition_table_descriptor_data_t;



sys_partition_t sys_partition_iter_start(void);



sys_partition_t sys_partition_iter_next(sys_partition_t partition);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_partition_get_data(sys_partition_t partition,sys_partition_data_t* out);



sys_partition_table_descriptor_t sys_partition_table_descriptor_iter_start(void);



sys_partition_table_descriptor_t sys_partition_table_descriptor_iter_next(sys_partition_table_descriptor_t partition_table_descriptor);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_partition_table_descriptor_get_data(sys_partition_table_descriptor_t partition_table_descriptor,sys_partition_table_descriptor_data_t* out);



#endif
