#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
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
		const partition_t* partition=handle->object;
		const drive_t* drive=partition->drive;
		char buffer[32];
		format_string(buffer,32,"%s%ud%up%u",drive->type->name,drive->controller_index,drive->device_index,partition->index);
		vfs_node_t* node=dynamicfs_create_node(_devfs_partition_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"id","%lu",HANDLE_ID_GET_INDEX(handle->rb_node.key));
		dynamicfs_create_node(node,"name",VFS_NODE_TYPE_FILE,smm_duplicate(partition->name),NULL,NULL);
		dynamicfs_create_data_node(node,"start_lba","%lu",partition->start_lba);
		dynamicfs_create_data_node(node,"end_lba","%lu",partition->end_lba);
		char path[64];
		format_string(path,64,"drive/%s%ud%u/partitions",drive->type->name,drive->controller_index,drive->device_index);
		dynamicfs_create_link_node(vfs_lookup(devfs->root,path,1),buffer,"../../../partition/%s",buffer);
		dynamicfs_create_link_node(devfs->root,buffer,"partition/%s",buffer);
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
	_devfs_partition_root=dynamicfs_create_node(devfs->root,"partition",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	handle_register_notification_listener(HANDLE_TYPE_PARTITION,&_devfs_partition_notification_listener);
}
