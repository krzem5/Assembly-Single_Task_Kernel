#include <devfs/fs.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/notification/notification.h>
#include <kernel/partition/partition.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_partition"



static vfs_node_t* _devfs_partition_root;



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		partition_t* partition=handle->object;
		drive_t* drive=partition->drive;
		char buffer[32];
		format_string(buffer,32,"%s%ud%up%u",drive->type->name,drive->controller_index,drive->device_index,partition->index);
		vfs_node_t* node=devfs_create_node(_devfs_partition_root,buffer,NULL);
		devfs_create_node(node,"name",smm_duplicate(partition->name));
		devfs_create_data_node(node,"start_lba","%lu",partition->start_lba);
		devfs_create_data_node(node,"end_lba","%lu",partition->end_lba);
		char path[64];
		format_string(path,64,"drive/%s%ud%u/partition",drive->type->name,drive->controller_index,drive->device_index);
		devfs_create_link_node(vfs_lookup(devfs->root,path,1),buffer,"../../../partition/%s",buffer);
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _devfs_partition_notification_listener={
	_listener
};



void devfs_partition_init(void){
	LOG("Creating partition subsystem...");
	_devfs_partition_root=devfs_create_node(devfs->root,"partition",NULL);
	handle_register_notification_listener(HANDLE_TYPE_PARTITION,&_devfs_partition_notification_listener);
}
