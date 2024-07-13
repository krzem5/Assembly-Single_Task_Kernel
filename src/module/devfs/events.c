#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/drive/drive.h>
#include <kernel/event/device.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/partition/partition.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_events"



static vfs_node_t* KERNEL_INIT_WRITE _devfs_drive_root=NULL;
static vfs_node_t* KERNEL_INIT_WRITE _devfs_partition_root=NULL;



static void _update_notification_thread(void){
	notification_consumer_t* consumer=notification_consumer_create(event_device_notification_dispatcher);
	while (1){
		notification_t notification;
		if (!notification_consumer_get(consumer,1,&notification)){
			continue;
		}
		if (notification.type==EVENT_DRIVE_CREATE_NOTIFICATION&&notification.length==sizeof(event_drive_create_notification_data_t)){
			const event_drive_create_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->drive_handle,drive_handle_type);
			if (!handle){
				continue;
			}
			const drive_t* drive=KERNEL_CONTAINEROF(handle,const drive_t,handle);
			char buffer[32];
			format_string(buffer,32,"%s%ud%u",drive->type->name,drive->controller_index,drive->device_index);
			vfs_node_t* node=dynamicfs_create_node(_devfs_drive_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_data_node(node,"id","%lu",HANDLE_ID_GET_INDEX(handle->rb_node.key)));
			vfs_node_unref(dynamicfs_create_node(node,"serial_number",VFS_NODE_TYPE_FILE,smm_duplicate(drive->serial_number),NULL,NULL));
			vfs_node_unref(dynamicfs_create_node(node,"model_number",VFS_NODE_TYPE_FILE,smm_duplicate(drive->model_number),NULL,NULL));
			vfs_node_unref(dynamicfs_create_data_node(node,"block_count","%lu",drive->block_count));
			vfs_node_unref(dynamicfs_create_data_node(node,"block_size","%lu",drive->block_size));
			vfs_node_unref(dynamicfs_create_node(node,"partitions",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL));
			vfs_node_unref(node);
			vfs_node_unref(dynamicfs_create_link_node(devfs->root,buffer,"drive/%s",buffer));
			handle_release(handle);
		}
		else if (notification.type==EVENT_DRIVE_DELETE_NOTIFICATION&&notification.length==sizeof(event_drive_delete_notification_data_t)){
			// ERROR("DELETE DRIVE");
		}
		else if (notification.type==EVENT_PARTITION_CREATE_NOTIFICATION&&notification.length==sizeof(event_partition_create_notification_data_t)){
			const event_partition_create_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->partition_handle,partition_handle_type);
			if (!handle){
				continue;
			}
			const partition_t* partition=KERNEL_CONTAINEROF(handle,const partition_t,handle);
			const drive_t* drive=partition->drive;
			char buffer[32];
			format_string(buffer,32,"%s%ud%up%u",drive->type->name,drive->controller_index,drive->device_index,partition->index);
			vfs_node_t* node=dynamicfs_create_node(_devfs_partition_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_data_node(node,"id","%lu",HANDLE_ID_GET_INDEX(handle->rb_node.key)));
			vfs_node_unref(dynamicfs_create_node(node,"name",VFS_NODE_TYPE_FILE,smm_duplicate(partition->name),NULL,NULL));
			vfs_node_unref(dynamicfs_create_data_node(node,"start_lba","%lu",partition->start_lba));
			vfs_node_unref(dynamicfs_create_data_node(node,"end_lba","%lu",partition->end_lba));
			vfs_node_unref(node);
			char path[64];
			format_string(path,64,"drive/%s%ud%u/partitions",drive->type->name,drive->controller_index,drive->device_index);
			vfs_node_unref(dynamicfs_create_link_node(vfs_lookup(devfs->root,path,VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0),buffer,"../../../partition/%s",buffer));
			vfs_node_unref(dynamicfs_create_link_node(devfs->root,buffer,"partition/%s",buffer));
			handle_release(handle);
		}
		else if (notification.type==EVENT_PARTITION_DELETE_NOTIFICATION&&notification.length==sizeof(event_partition_delete_notification_data_t)){
			// ERROR("DELETE PARTITION");
		}
	}
}



MODULE_POSTINIT(){
	LOG("Creating device subsystem...");
	_devfs_drive_root=dynamicfs_create_node(devfs->root,"drive",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	LOG("Creating partition subsystem...");
	_devfs_partition_root=dynamicfs_create_node(devfs->root,"partition",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	LOG("Starting kernel event listener...");
	thread_create_kernel_thread(NULL,"devfs.update",_update_notification_thread,0);
}
