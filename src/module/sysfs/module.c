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



static vfs_node_t* KERNEL_INIT_WRITE _sysfs_module_type_root=NULL;



static u64 _sysfs_module_state_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	handle_t* handle=handle_lookup_and_acquire((u64)ctx,module_handle_type);
	const char* state="Unknown";
	if (handle){
		const module_t* module=KERNEL_CONTAINEROF(handle,const module_t,handle);
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
		handle_release(handle);
	}
	return dynamicfs_process_simple_read(state,smm_length(state),offset,buffer,size);
}



static void _update_notification_thread(void){
	notification_consumer_t* consumer=notification_consumer_create(module_notification_dispatcher);
	while (1){
		notification_t notification;
		if (!notification_consumer_get(consumer,1,&notification)){
			continue;
		}
		if (notification.type==MODULE_LOAD_NOTIFICATION&&notification.length>=sizeof(module_load_notification_data_t)){
			const module_load_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->module_handle,module_handle_type);
			if (!handle){
				continue;
			}
			const module_t* module=KERNEL_CONTAINEROF(handle,const module_t,handle);
			vfs_node_t* node=dynamicfs_create_node(_sysfs_module_type_root,module->name->data,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_link_node(node,"exe","/boot/module/%s.mod",module->name->data));
			vfs_node_unref(dynamicfs_create_node(node,"state",VFS_NODE_TYPE_FILE,NULL,_sysfs_module_state_read_callback,(void*)(data->module_handle)));
			vfs_node_unref(node);
			handle_release(handle);
		}
		else if (notification.type==MODULE_UNLOAD_NOTIFICATION&&notification.length>=sizeof(module_unload_notification_data_t)){
			const module_unload_notification_data_t* data=notification.data;
			vfs_node_t* node=vfs_lookup(_sysfs_module_type_root,data->name,0,0,0);
			if (!node){
				continue;
			}
			dynamicfs_delete_node(vfs_lookup(node,"exe",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"state",0,0,0),1);
			dynamicfs_delete_node(node,0);
		}
	}
}



MODULE_POSTINIT(){
	LOG("Creating module subsystem...");
	_sysfs_module_type_root=dynamicfs_create_node(sysfs->root,"module",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	LOG("Starting module event listener...");
	thread_create_kernel_thread(NULL,"sysfs.module.update",_update_notification_thread,0);
}
