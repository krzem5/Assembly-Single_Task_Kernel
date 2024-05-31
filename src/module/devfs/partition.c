#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/partition/partition.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_partition"



static vfs_node_t* _devfs_partition_root;



static void _listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,partition_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const partition_t* partition=KERNEL_CONTAINEROF(handle,const partition_t,handle);
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
		dynamicfs_create_link_node(vfs_lookup(devfs->root,path,VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0),buffer,"../../../partition/%s",buffer);
		dynamicfs_create_link_node(devfs->root,buffer,"partition/%s",buffer);
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
	handle_release(handle);
}



static void _update_notification_thread(void){
	(void)_listener;
	// notification_consumer_t* consumer=notification_consumer_create(&(handle_get_descriptor(partition_handle_type)->notification_dispatcher));
	// HANDLE_FOREACH(partition_handle_type){
	// 	_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	// }
	// while (1){
	// 	notification_t notification;
	// 	if (!notification_consumer_get(consumer,1,&notification)){
	// 		continue;
	// 	}
	// 	handle_t* handle=handle_lookup_and_acquire(notification.object,partition_handle_type);
	// 	if (!handle){
	// 		continue;
	// 	}
	// 	_listener(handle->rb_node.key,notification.type);
	// 	handle_release(handle);
	// }
}



MODULE_POSTPOSTINIT(){
	LOG("Creating partition subsystem...");
	_devfs_partition_root=dynamicfs_create_node(devfs->root,"partition",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	thread_create_kernel_thread(NULL,"devfs.partition.update.notification",_update_notification_thread,0);
}
