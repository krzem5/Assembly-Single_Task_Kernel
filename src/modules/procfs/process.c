#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <procfs/fs.h>
#define KERNEL_LOG_NAME "procfs_process"



vfs_node_t* procfs_process_root;



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const process_t* process=handle->object;
		char buffer[32];
		format_string(buffer,32,"%lu",HANDLE_ID_GET_INDEX(process->handle.rb_node.key));
		vfs_node_t* node=dynamicfs_create_node(procfs_process_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_node(node,"threads",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _procfs_process_notification_listener={
	_listener
};



static u64 _process_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",HANDLE_ID_GET_INDEX(THREAD_DATA->process->handle.rb_node.key)),offset,buffer,size);
}



void procfs_process_init(void){
	LOG("Creating process subsystem...");
	procfs_process_root=dynamicfs_create_node(procfs->root,"process",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_create_node(procfs_process_root,"self",VFS_NODE_TYPE_LINK,NULL,_process_self_read_callback,NULL);
	handle_register_notification_listener(HANDLE_TYPE_PROCESS,&_procfs_process_notification_listener);
}
