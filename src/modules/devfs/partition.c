#include <devfs/fs.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/partition/partition.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_partition"



void devfs_partition_init(void){
	LOG("Creating partition subsystem...");
	vfs_node_t* root=devfs_create_node(devfs->root,"partition",NULL);
	HANDLE_FOREACH(HANDLE_TYPE_PARTITION){
		handle_acquire(handle);
		partition_t* partition=handle->object;
		drive_t* drive=partition->drive;
		char buffer[32];
		format_string(buffer,32,"%s%ud%up%u",drive->type->name,drive->controller_index,drive->device_index,partition->index);
		vfs_node_t* node=devfs_create_node(root,buffer,NULL);
		devfs_create_node(node,"name",smm_duplicate(partition->name));
		devfs_create_data_node(node,"start_lba","%lu",partition->start_lba);
		devfs_create_data_node(node,"end_lba","%lu",partition->end_lba);
		char path[64];
		format_string(path,64,"drive/%s%ud%u/partition",drive->type->name,drive->controller_index,drive->device_index);
		node=vfs_lookup(devfs->root,path,1);
		devfs_create_link_node(node,buffer,"../../../partition/%s",buffer);
		handle_release(handle);
	}
}
