#include <dynamicfs/dynamicfs.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_module"



static vfs_node_t* KERNEL_INIT_WRITE _sysfs_module_type_root;



static u64 _sysfs_module_state_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	const module_t* module=ctx;
	const char* state="Unknown";
	switch (module->state){
		case MODULE_STATE_LOADING:
			state="Loading";
			break;
		case MODULE_STATE_LOADED:
			state="Loaded";
			break;
		case MODULE_STATE_UNLOADING:
			state="Unloading";
			break;
		case MODULE_STATE_UNLOADED:
			state="Unloaded";
			break;
	}
	return dynamicfs_process_simple_read(state,smm_length(state),offset,buffer,size);
}



static void _listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,module_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const module_t* module=KERNEL_CONTAINEROF(handle,const module_t,handle);
		vfs_node_t* node=dynamicfs_create_node(_sysfs_module_type_root,module->name->data,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_link_node(node,"exe","/boot/module/%s.mod",module->name->data);
		dynamicfs_create_node(node,"state",VFS_NODE_TYPE_FILE,NULL,_sysfs_module_state_read_callback,(void*)module);
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		const module_t* module=KERNEL_CONTAINEROF(handle,const module_t,handle);
		vfs_node_t* node=vfs_lookup(_sysfs_module_type_root,module->name->data,0,0,0);
		if (node){
			dynamicfs_delete_node(vfs_lookup(node,"exe",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"state",0,0,0),1);
			dynamicfs_delete_node(node,0);
		}
	}
	handle_release(handle);
}



static void _update_notification_thread(void){
	notification2_consumer_t* consumer=notification2_consumer_create(&(handle_get_descriptor(module_handle_type)->notification_dispatcher));
	HANDLE_FOREACH(module_handle_type){
		_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	}
	while (1){
		notification2_t notification;
		if (!notification2_consumer_get(consumer,1,&notification)){
			continue;
		}
		handle_t* handle=handle_lookup_and_acquire(notification.object,module_handle_type);
		if (!handle){
			continue;
		}
		_listener(handle->rb_node.key,notification.type);
		handle_release(handle);
	}
}



MODULE_POSTINIT(){
	LOG("Creating module subsystem...");
	_sysfs_module_type_root=dynamicfs_create_node(sysfs->root,"module",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	thread_create_kernel_thread(NULL,"sysfs.module.update.notification",_update_notification_thread,0);
}
