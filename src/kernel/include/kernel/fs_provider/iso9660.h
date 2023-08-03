#ifndef _KERNEL_FS_PROVIDER_ISO9660_H_
#define _KERNEL_FS_PROVIDER_ISO9660_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>



void iso9660_load(const drive_t* drive,const fs_partition_config_t* partition_config,u32 block_index,u32 data_length);



#endif
