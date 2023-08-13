#ifndef _KERNEL_FS_EMPTYFS_H_
#define _KERNEL_FS_EMPTYFS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>



void emptyfs_load(const drive_t* drive,const partition_config_t* partition_config);



#endif
