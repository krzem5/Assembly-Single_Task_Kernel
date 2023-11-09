#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_drive"



static vfs_node_t* _devfs_drive_root;



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const drive_t* drive=handle->object;
		char buffer[32];
		format_string(buffer,32,"%s%ud%u",drive->type->name,drive->controller_index,drive->device_index);
		vfs_node_t* node=dynamicfs_create_node(_devfs_drive_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_node(node,"serial_number",VFS_NODE_TYPE_FILE,smm_duplicate(drive->serial_number),NULL,NULL);
		dynamicfs_create_node(node,"model_number",VFS_NODE_TYPE_FILE,smm_duplicate(drive->model_number),NULL,NULL);
		dynamicfs_create_data_node(node,"block_count","%lu",drive->block_count);
		dynamicfs_create_data_node(node,"block_size","%lu",drive->block_size);
		dynamicfs_create_node(node,"partition",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _devfs_drive_notification_listener={
	_listener
};



void devfs_drive_init(void){
	LOG("Creating drive subsystem...");
	_devfs_drive_root=dynamicfs_create_node(devfs->root,"drive",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	handle_register_notification_listener(HANDLE_TYPE_DRIVE,&_devfs_drive_notification_listener);
}
