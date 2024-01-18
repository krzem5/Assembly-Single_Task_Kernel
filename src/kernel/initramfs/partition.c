#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "initramfs_partition"



static _Bool _initramfs_init_partitions(drive_t* drive){
	if (!streq(drive->type->name,"initramfs")){
		return 0;
	}
	partition_t* partition=partition_create(drive,0,"initramfs",0,drive->block_count);
	if (!partition->fs){
		return 0;
	}
	vfs_mount(partition->fs,NULL,0);
	return 1;
}



static const partition_table_descriptor_config_t _initramfs_partition_table_descriptor={
	"initramfs",
	_initramfs_init_partitions
};



void KERNEL_EARLY_EXEC initramfs_partition_init(void){
	LOG("Registering initramfs partition descriptor...");
	partition_register_table_descriptor(&_initramfs_partition_table_descriptor);
}
