#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <procfs/fs.h>
#include <procfs/process.h>
#define KERNEL_LOG_NAME "procfs_thread"



static vfs_node_t* _procfs_thread_root;



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const thread_t* thread=handle->object;
		char buffer[32];
		format_string(buffer,32,"%lu/threads",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key));
		vfs_node_t* root=vfs_lookup(procfs_process_root,buffer,0);
		if (!root){
			return;
		}
		format_string(buffer,32,"%lu",HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"name","thread-%lu",HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
		dynamicfs_create_link_node(_procfs_thread_root,buffer,"../process/%lu/threads/%lu",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key),HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle->rb_node.key);
	}
}



static notification_listener_t _procfs_thread_notification_listener={
	_listener
};



static u64 _thread_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",HANDLE_ID_GET_INDEX(THREAD_DATA->handle.rb_node.key)),offset,buffer,size);
}



void procfs_thread_init(void){
	LOG("Creating thread subsystem...");
	_procfs_thread_root=dynamicfs_create_node(procfs->root,"thread",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_create_node(_procfs_thread_root,"self",VFS_NODE_TYPE_LINK,NULL,_thread_self_read_callback,NULL);
	handle_register_notification_listener(HANDLE_TYPE_THREAD,&_procfs_thread_notification_listener);
}
