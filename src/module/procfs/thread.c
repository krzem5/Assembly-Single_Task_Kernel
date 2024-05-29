#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <procfs/fs.h>
#define KERNEL_LOG_NAME "procfs_thread"



static vfs_node_t* KERNEL_INIT_WRITE _procfs_thread_root=NULL;



static void _listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,thread_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const thread_t* thread=handle->object;
		char buffer[64];
		format_string(buffer,64,"%lu/threads",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key));
		vfs_node_t* root=vfs_lookup(procfs->root,buffer,0,0,0);
		if (!root){
			return;
		}
		format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_node(node,"name",VFS_NODE_TYPE_FILE,thread->name,NULL,NULL);
		dynamicfs_create_link_node(_procfs_thread_root,buffer,"../%lu/threads/%lu",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key),HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		const thread_t* thread=handle->object;
		char buffer[64];
		format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
		dynamicfs_delete_node(vfs_lookup(_procfs_thread_root,buffer,0,0,0),1);
		format_string(buffer,64,"%lu/threads/%lu",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key),HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
		vfs_node_t* node=vfs_lookup(procfs->root,buffer,0,0,0);
		dynamicfs_delete_node(vfs_lookup(node,"name",0,0,0),0);
		dynamicfs_delete_node(node,0);
	}
	handle_release(handle);
}



static u64 _thread_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",(THREAD_DATA->header.current_thread?HANDLE_ID_GET_INDEX(THREAD_DATA->handle.rb_node.key):0)),offset,buffer,size);
}



static void _update_notification_thread(void){
	notification2_consumer_t* consumer=notification2_consumer_create(&(handle_get_descriptor(thread_handle_type)->notification_dispatcher));
	HANDLE_FOREACH(thread_handle_type){
		_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	}
	while (1){
		notification2_t notification;
		if (!notification2_consumer_get(consumer,1,&notification)){
			continue;
		}
		handle_t* handle=handle_lookup_and_acquire(notification.object,thread_handle_type);
		if (!handle){
			continue;
		}
		_listener(handle->rb_node.key,notification.type);
		handle_release(handle);
	}
}



MODULE_POSTPOSTINIT(){
	LOG("Creating thread subsystem...");
	_procfs_thread_root=dynamicfs_create_node(procfs->root,"thread",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_create_node(_procfs_thread_root,"self",VFS_NODE_TYPE_LINK,NULL,_thread_self_read_callback,NULL);
	// thread_create_kernel_thread(NULL,"procfs.thread.update.notification",_update_notification_thread,0);
	(void)_update_notification_thread;
}
