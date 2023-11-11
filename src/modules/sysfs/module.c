#include <dynamicfs/dynamicfs.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/notification/notification.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_module"



static vfs_node_t* _sysfs_module_type_root;



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



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const module_t* module=handle->object;
		vfs_node_t* node=dynamicfs_create_node(_sysfs_module_type_root,module->descriptor->name,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_link_node(node,"exe","/boot/module/%s.mod",module->descriptor->name);
		dynamicfs_create_node(node,"state",VFS_NODE_TYPE_FILE,NULL,_sysfs_module_state_read_callback,(void*)module);
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _sysfs_module_notification_listener={
	_listener
};



void sysfs_module_init(void){
	LOG("Creating module subsystem...");
	_sysfs_module_type_root=dynamicfs_create_node(sysfs->root,"module",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	handle_register_notification_listener(HANDLE_TYPE_MODULE,&_sysfs_module_notification_listener);
}
