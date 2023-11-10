#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/notification/notification.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_handle"



static vfs_node_t* _sysfs_handle_type_root;



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const handle_descriptor_t* descriptor=handle->object;
		char buffer[64];
		strcpy_lowercase(buffer,descriptor->name,64);
		vfs_node_t* node=dynamicfs_create_node(_sysfs_handle_type_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_node(node,"count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(descriptor->active_count)));
		dynamicfs_create_node(node,"lifetime_count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(descriptor->count)));
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _sysfs_handle_notification_listener={
	_listener
};



void sysfs_handle_init(void){
	LOG("Creating handle subsystem...");
	_sysfs_handle_type_root=dynamicfs_create_node(sysfs->root,"handle",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	handle_register_notification_listener(HANDLE_TYPE_HANDLE,&_sysfs_handle_notification_listener);
}
