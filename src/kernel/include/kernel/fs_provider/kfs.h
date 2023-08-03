#ifndef _KERNEL_FS_PROVIDER_KFS_H_
#define _KERNEL_FS_PROVIDER_KFS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>



#define KFS_SIGNATURE 0x2053464b2053464bull



void kfs_load(const drive_t* drive,const partition_config_t* partition_config);



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length);



#endif
