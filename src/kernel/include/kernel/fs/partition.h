#ifndef _KERNEL_FS_PARTITION_H_
#define _KERNEL_FS_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/types.h>



#define FS_PARTITION_TYPE_DRIVE 0
#define FS_PARTITION_TYPE_EMPTY 1
#define FS_PARTITION_TYPE_ISO9660 2
#define FS_PARTITION_TYPE_GPT 3



typedef struct _FS_PARTITION_CONFIG{
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
} fs_partition_config_t;



void fs_partition_load_from_drive(drive_t* drive);



#endif
