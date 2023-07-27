#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



void kfs_load(drive_t* drive,const fs_partition_config_t* partition_config){
	//
}



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length){
	LOG("Formatting drive '%s' as KFS...",drive->model_number);
	if (drive->block_size!=512){
		WARN("KFS requires block_size to be equal to 512 bytes");
		return 0;
	}
	return 0;
}
