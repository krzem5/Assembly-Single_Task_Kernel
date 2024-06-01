#include <dynamicfs/dynamicfs.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_handle"



#define NOTIFICATION_TYPE_HANDLE_CREATE 1
#define NOTIFICATION_TYPE_HANDLE_DELETE 2



static vfs_node_t* KERNEL_INIT_WRITE _sysfs_handle_type_root;



static void _listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,handle_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const handle_descriptor_t* descriptor=KERNEL_CONTAINEROF(handle,const handle_descriptor_t,handle);
		vfs_node_t* node=dynamicfs_create_node(_sysfs_handle_type_root,descriptor->name,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_set_root_only(node);
		dynamicfs_set_root_only(dynamicfs_create_node(node,"count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(descriptor->active_count))));
		dynamicfs_set_root_only(dynamicfs_create_node(node,"lifetime_count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(descriptor->count))));
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
	handle_release(handle);
}



static void _update_notification_thread(void){
	(void)_listener;
	// notification_consumer_t* consumer=notification_consumer_create(&(handle_get_descriptor(handle_handle_type)->notification_dispatcher));
	// HANDLE_FOREACH(handle_handle_type){
	// 	_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	// }
	// while (1){
	// 	notification_t notification;
	// 	if (!notification_consumer_get(consumer,1,&notification)){
	// 		continue;
	// 	}
	// 	handle_t* handle=handle_lookup_and_acquire(notification.object,handle_handle_type);
	// 	if (!handle){
	// 		continue;
	// 	}
	// 	_listener(handle->rb_node.key,notification.type);
	// 	handle_release(handle);
	// }
}



MODULE_POSTINIT(){
	LOG("Creating handle subsystem...");
	_sysfs_handle_type_root=dynamicfs_create_node(sysfs->root,"handle",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(_sysfs_handle_type_root);
	thread_create_kernel_thread(NULL,"sysfs.handle.update.notification",_update_notification_thread,0);
}
