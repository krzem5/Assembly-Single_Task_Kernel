#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_drive"



static vfs_node_t* _devfs_drive_root;



static void _listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,drive_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const drive_t* drive=KERNEL_CONTAINEROF(handle,const drive_t,handle);
		char buffer[32];
		format_string(buffer,32,"%s%ud%u",drive->type->name,drive->controller_index,drive->device_index);
		vfs_node_t* node=dynamicfs_create_node(_devfs_drive_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"id","%lu",HANDLE_ID_GET_INDEX(handle->rb_node.key));
		dynamicfs_create_node(node,"serial_number",VFS_NODE_TYPE_FILE,smm_duplicate(drive->serial_number),NULL,NULL);
		dynamicfs_create_node(node,"model_number",VFS_NODE_TYPE_FILE,smm_duplicate(drive->model_number),NULL,NULL);
		dynamicfs_create_data_node(node,"block_count","%lu",drive->block_count);
		dynamicfs_create_data_node(node,"block_size","%lu",drive->block_size);
		dynamicfs_create_node(node,"partitions",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_link_node(devfs->root,buffer,"drive/%s",buffer);
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
	handle_release(handle);
}



static void _update_notification_thread(void){
	notification2_consumer_t* consumer=notification2_consumer_create(&(handle_get_descriptor(drive_handle_type)->notification_dispatcher));
	HANDLE_FOREACH(drive_handle_type){
		_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	}
	while (1){
		notification2_t notification;
		if (!notification2_consumer_get(consumer,1,&notification)){
			continue;
		}
		handle_t* handle=handle_lookup_and_acquire(notification.object,drive_handle_type);
		if (!handle){
			continue;
		}
		_listener(handle->rb_node.key,notification.type);
		handle_release(handle);
	}
}



MODULE_POSTINIT(){
	LOG("Creating drive subsystem...");
	_devfs_drive_root=dynamicfs_create_node(devfs->root,"drive",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	thread_create_kernel_thread(NULL,"devfs.drive.update.notification",_update_notification_thread,0);
}
