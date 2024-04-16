#ifndef _SYS_DRIVE_DRIVE_H_
#define _SYS_DRIVE_DRIVE_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_drive_t;



typedef struct _SYS_DRIVE_DATA{
	char type[64];
	u16 controller_index;
	u16 device_index;
	char serial_number[256];
	char model_number[256];
	u64 block_count;
	u64 block_size;
	char partition_table_type[64];
} sys_drive_data_t;



sys_drive_t sys_drive_iter_start(void);



sys_drive_t sys_drive_iter_next(sys_drive_t drive);



sys_error_t __attribute__((access(write_only,2),nonnull)) sys_drive_get_data(sys_drive_t drive,sys_drive_data_t* out);



#endif
