#include <dynamicfs/dynamicfs.h>
#include <kernel/event/process.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <procfs/fs.h>
#define KERNEL_LOG_NAME "procfs_events"



static vfs_node_t* KERNEL_INIT_WRITE _procfs_group_root=NULL;
static vfs_node_t* KERNEL_INIT_WRITE _procfs_thread_root=NULL;



static u64 _process_read_stdin_callback(void* ctx,u64 offset,void* buffer,u64 size){
	handle_t* handle=handle_lookup_and_acquire((u64)ctx,process_handle_type);
	char link_buffer[4096]="";
	u32 link_buffer_length=0;
	if (handle){
		const process_t* process=KERNEL_CONTAINEROF(handle,const process_t,handle);
		vfs_node_t* node=fd_get_node(process->fd_stdin,NULL);
		if (node){
			link_buffer_length=vfs_path(node,link_buffer,sizeof(link_buffer));
			vfs_node_unref(node);
		}
		handle_release(handle);
	}
	return dynamicfs_process_simple_read(link_buffer,link_buffer_length,offset,buffer,size);
}



static u64 _process_read_stdout_callback(void* ctx,u64 offset,void* buffer,u64 size){
	handle_t* handle=handle_lookup_and_acquire((u64)ctx,process_handle_type);
	char link_buffer[4096]="";
	u32 link_buffer_length=0;
	if (handle){
		const process_t* process=KERNEL_CONTAINEROF(handle,const process_t,handle);
		vfs_node_t* node=fd_get_node(process->fd_stdout,NULL);
		if (node){
			link_buffer_length=vfs_path(node,link_buffer,sizeof(link_buffer));
			vfs_node_unref(node);
		}
		handle_release(handle);
	}
	return dynamicfs_process_simple_read(link_buffer,link_buffer_length,offset,buffer,size);
}



static u64 _process_read_stderr_callback(void* ctx,u64 offset,void* buffer,u64 size){
	handle_t* handle=handle_lookup_and_acquire((u64)ctx,process_handle_type);
	char link_buffer[4096]="";
	u32 link_buffer_length=0;
	if (handle){
		const process_t* process=KERNEL_CONTAINEROF(handle,const process_t,handle);
		vfs_node_t* node=fd_get_node(process->fd_stderr,NULL);
		if (node){
			link_buffer_length=vfs_path(node,link_buffer,sizeof(link_buffer));
			vfs_node_unref(node);
		}
		handle_release(handle);
	}
	return dynamicfs_process_simple_read(link_buffer,link_buffer_length,offset,buffer,size);
}



static u64 _process_read_group_callback(void* ctx,u64 offset,void* buffer,u64 size){
	handle_t* handle=handle_lookup_and_acquire((u64)ctx,process_handle_type);
	char group_buffer[4096]="";
	u32 group_buffer_length=0;
	if (handle){
		const process_t* process=KERNEL_CONTAINEROF(handle,const process_t,handle);
		group_buffer_length=format_string(group_buffer,sizeof(group_buffer),"../group/%lu",(process->process_group?HANDLE_ID_GET_INDEX(process->process_group->handle.rb_node.key):0));
		handle_release(handle);
	}
	return dynamicfs_process_simple_read(group_buffer,group_buffer_length,offset,buffer,size);
}



static u64 _process_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",(THREAD_DATA->header.current_thread?HANDLE_ID_GET_INDEX(THREAD_DATA->process->handle.rb_node.key):0)),offset,buffer,size);
}



static u64 _group_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",(THREAD_DATA->header.current_thread?HANDLE_ID_GET_INDEX(THREAD_DATA->process->process_group->handle.rb_node.key):0)),offset,buffer,size);
}



static u64 _thread_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",(THREAD_DATA->header.current_thread?HANDLE_ID_GET_INDEX(THREAD_DATA->handle.rb_node.key):0)),offset,buffer,size);
}



static void _update_notification_thread(void){
	notification_consumer_t* consumer=notification_consumer_create(event_process_notification_dispatcher);
	while (1){
		notification_t notification;
		if (!notification_consumer_get(consumer,1,&notification)){
			continue;
		}
		if (notification.type==EVENT_PROCESS_CREATE_NOTIFICATION&&notification.length==sizeof(event_process_create_notification_data_t)){
			const event_process_create_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->process_handle,process_handle_type);
			if (!handle){
				continue;
			}
			const process_t* process=KERNEL_CONTAINEROF(handle,const process_t,handle);
			char buffer[32];
			format_string(buffer,32,"%lu",HANDLE_ID_GET_INDEX(process->handle.rb_node.key));
			vfs_node_t* node=dynamicfs_create_node(procfs->root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_node(node,"name",VFS_NODE_TYPE_FILE,NULL,dynamicfs_string_read_callback,(void*)(&(process->name))));
			vfs_node_unref(dynamicfs_create_node(node,"exe",VFS_NODE_TYPE_LINK,process->image,NULL,NULL));
			vfs_node_unref(dynamicfs_create_node(node,"stdin",VFS_NODE_TYPE_LINK,NULL,_process_read_stdin_callback,(void*)(data->process_handle)));
			vfs_node_unref(dynamicfs_create_node(node,"stdout",VFS_NODE_TYPE_LINK,NULL,_process_read_stdout_callback,(void*)(data->process_handle)));
			vfs_node_unref(dynamicfs_create_node(node,"stderr",VFS_NODE_TYPE_LINK,NULL,_process_read_stderr_callback,(void*)(data->process_handle)));
			vfs_node_unref(dynamicfs_create_node(node,"threads",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL));
			vfs_node_unref(dynamicfs_create_node(node,"group",VFS_NODE_TYPE_LINK,NULL,_process_read_group_callback,(void*)(data->process_handle)));
			vfs_node_unref(node);
			handle_release(handle);
		}
		else if (notification.type==EVENT_PROCESS_DELETE_NOTIFICATION&&notification.length==sizeof(event_process_delete_notification_data_t)){
			const event_process_delete_notification_data_t* data=notification.data;
			char buffer[32];
			format_string(buffer,32,"%lu",HANDLE_ID_GET_INDEX(data->process_handle));
			vfs_node_t* node=vfs_lookup(procfs->root,buffer,0,0,0);
			if (!node){
				continue;
			}
			dynamicfs_delete_node(vfs_lookup(node,"exe",0,0,0),0);
			dynamicfs_delete_node(vfs_lookup(node,"name",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"stderr",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"stdin",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"stdout",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"threads",0,0,0),1);
			dynamicfs_delete_node(vfs_lookup(node,"group",0,0,0),1);
			dynamicfs_delete_node(node,0);
		}
		else if (notification.type==EVENT_THREAD_CREATE_NOTIFICATION&&notification.length==sizeof(event_thread_create_notification_data_t)){
			const event_thread_create_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->thread_handle,thread_handle_type);
			if (!handle){
				continue;
			}
			const thread_t* thread=KERNEL_CONTAINEROF(handle,const thread_t,handle);
			char buffer[64];
			format_string(buffer,64,"%lu/threads",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key));
			vfs_node_t* root=vfs_lookup(procfs->root,buffer,0,0,0);
			format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(thread->handle.rb_node.key));
			vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(root);
			vfs_node_unref(dynamicfs_create_node(node,"name",VFS_NODE_TYPE_FILE,thread->name,NULL,NULL));
			vfs_node_unref(node);
			vfs_node_unref(dynamicfs_create_link_node(_procfs_thread_root,buffer,"../%lu/threads/%lu",HANDLE_ID_GET_INDEX(thread->process->handle.rb_node.key),HANDLE_ID_GET_INDEX(thread->handle.rb_node.key)));
			handle_release(handle);
		}
		else if (notification.type==EVENT_THREAD_DELETE_NOTIFICATION&&notification.length==sizeof(event_thread_delete_notification_data_t)){
			const event_thread_delete_notification_data_t* data=notification.data;
			char buffer[64];
			format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(data->thread_handle));
			dynamicfs_delete_node(vfs_lookup(_procfs_thread_root,buffer,0,0,0),1);
			format_string(buffer,64,"%lu/threads/%lu",HANDLE_ID_GET_INDEX(data->process_handle),HANDLE_ID_GET_INDEX(data->thread_handle));
			vfs_node_t* node=vfs_lookup(procfs->root,buffer,0,0,0);
			if (!node){
				continue;
			}
			dynamicfs_delete_node(vfs_lookup(node,"name",0,0,0),0);
			dynamicfs_delete_node(node,0);
		}
		else if (notification.type==EVENT_PROCESS_GROUP_CREATE_NOTIFICATION&&notification.length==sizeof(event_process_group_create_notification_data_t)){
			const event_process_group_create_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->process_group_handle,process_group_handle_type);
			if (!handle){
				continue;
			}
			const process_group_t* process_group=KERNEL_CONTAINEROF(handle,const process_group_t,handle);
			char buffer[64];
			format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(process_group->handle.rb_node.key));
			vfs_node_t* node=dynamicfs_create_node(_procfs_group_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_node(node,"children",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL));
			vfs_node_unref(node);
			handle_release(handle);
		}
		else if (notification.type==EVENT_PROCESS_GROUP_JOIN_NOTIFICATION&&notification.length==sizeof(event_process_group_join_notification_data_t)){
			const event_process_group_join_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->process_group_handle,process_group_handle_type);
			if (!handle){
				continue;
			}
			const process_group_t* process_group=KERNEL_CONTAINEROF(handle,const process_group_t,handle);
			char buffer[64];
			format_string(buffer,64,"%lu/children",HANDLE_ID_GET_INDEX(process_group->handle.rb_node.key));
			vfs_node_t* root=vfs_lookup(_procfs_group_root,buffer,0,0,0);
			format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(data->process_handle));
			vfs_node_unref(dynamicfs_create_link_node(root,buffer,"../../../%lu",HANDLE_ID_GET_INDEX(data->process_handle)));
			vfs_node_unref(root);
			handle_release(handle);
		}
		else if (notification.type==EVENT_PROCESS_GROUP_LEAVE_NOTIFICATION&&notification.length==sizeof(event_process_group_leave_notification_data_t)){
			const event_process_group_leave_notification_data_t* data=notification.data;
			handle_t* handle=handle_lookup_and_acquire(data->process_group_handle,process_group_handle_type);
			if (!handle){
				continue;
			}
			const process_group_t* process_group=KERNEL_CONTAINEROF(handle,const process_group_t,handle);
			char buffer[64];
			format_string(buffer,64,"%lu/children",HANDLE_ID_GET_INDEX(process_group->handle.rb_node.key));
			vfs_node_t* root=vfs_lookup(_procfs_group_root,buffer,0,0,0);
			format_string(buffer,64,"%lu",HANDLE_ID_GET_INDEX(data->process_handle));
			vfs_node_t* node=vfs_lookup(root,buffer,0,0,0);
			if (node){
				dynamicfs_delete_node(node,0);
			}
			vfs_node_unref(root);
			handle_release(handle);
		}
		else if (notification.type==EVENT_PROCESS_GROUP_DELETE_NOTIFICATION&&notification.length==sizeof(event_process_group_delete_notification_data_t)){
			ERROR("EVENT_PROCESS_GROUP_DELETE_NOTIFICATION");
		}
	}
}



MODULE_POSTPOSTINIT(){
	LOG("Creating process subsystem...");
	vfs_node_unref(dynamicfs_create_node(procfs->root,"self",VFS_NODE_TYPE_LINK,NULL,_process_self_read_callback,NULL));
	LOG("Creating group subsystem...");
	_procfs_group_root=dynamicfs_create_node(procfs->root,"group",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	vfs_node_unref(dynamicfs_create_node(_procfs_group_root,"self",VFS_NODE_TYPE_LINK,NULL,_group_self_read_callback,NULL));
	LOG("Creating thread subsystem...");
	_procfs_thread_root=dynamicfs_create_node(procfs->root,"thread",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	vfs_node_unref(dynamicfs_create_node(_procfs_thread_root,"self",VFS_NODE_TYPE_LINK,NULL,_thread_self_read_callback,NULL));
	LOG("Starting kernel event listener...");
	thread_create_kernel_thread(NULL,"procfs.update",_update_notification_thread,0);
}
