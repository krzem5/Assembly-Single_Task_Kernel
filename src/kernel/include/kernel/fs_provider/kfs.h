#ifndef _KERNEL_FS_PROVIDER_KFS_H_
#define _KERNEL_FS_PROVIDER_KFS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>



void kfs_load(drive_t* drive,const fs_partition_config_t* partition_config);



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length);



#endif
