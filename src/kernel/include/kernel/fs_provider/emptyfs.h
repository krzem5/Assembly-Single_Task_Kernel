#ifndef _KERNEL_FS_PROVIDER_EMPTYFS_H_
#define _KERNEL_FS_PROVIDER_EMPTYFS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>



void fs_emptyfs_load(drive_t* drive,const fs_partition_config_t* partition_config);



#endif
