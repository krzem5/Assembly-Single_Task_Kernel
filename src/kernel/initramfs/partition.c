#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "initramfs_partition"



static partition_table_descriptor_t* _initramfs_partition_table_descriptor=NULL;
static partition_t* _initramfs_partition=NULL;



static _Bool _initramfs_init_partitions(drive_t* drive){
	if (!streq(drive->type->name,"initramfs")){
		return 0;
	}
	partition_t* partition=partition_create(drive,0,"initramfs",0,drive->block_count);
	if (!partition->fs){
		handle_release(&(partition->handle));
		return 0;
	}
	_initramfs_partition=partition;
	vfs_mount(partition->fs,NULL,0);
	return 1;
}



static const partition_table_descriptor_config_t _initramfs_partition_table_descriptor_config={
	"initramfs",
	_initramfs_init_partitions
};



void KERNEL_EARLY_EXEC initramfs_partition_init(void){
	INFO("Registering initramfs partition descriptor...");
	_initramfs_partition_table_descriptor=partition_register_table_descriptor(&_initramfs_partition_table_descriptor_config);
}



void initramfs_partition_deinit(void){
	INFO("Unregistering initramfs partition descriptor...");
	if (_initramfs_partition){
		handle_release(&(_initramfs_partition->handle));
		_initramfs_partition=NULL;
	}
	if (_initramfs_partition_table_descriptor){
		partition_unregister_table_descriptor(_initramfs_partition_table_descriptor);
		_initramfs_partition_table_descriptor=NULL;
	}
}
