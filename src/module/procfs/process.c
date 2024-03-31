#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <procfs/fs.h>
#define KERNEL_LOG_NAME "procfs_process"



static void _listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const process_t* process=handle->object;
		char buffer[32];
		format_string(buffer,32,"%lu",HANDLE_ID_GET_INDEX(process->handle.rb_node.key));
		vfs_node_t* node=dynamicfs_create_node(procfs->root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_node(node,"name",VFS_NODE_TYPE_FILE,NULL,dynamicfs_string_read_callback,(void*)(&(process->name)));
		dynamicfs_create_node(node,"exe",VFS_NODE_TYPE_LINK,process->image,NULL,NULL);
		dynamicfs_create_link_node(node,"stdin","/dev/ser/in");
		dynamicfs_create_link_node(node,"stdout","/dev/ser/out");
		dynamicfs_create_link_node(node,"stderr","/dev/ser/out");
		dynamicfs_create_node(node,"threads",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		const process_t* process=handle->object;
		char buffer[32];
		format_string(buffer,32,"%lu",HANDLE_ID_GET_INDEX(process->handle.rb_node.key));
		vfs_node_t* node=vfs_lookup(procfs->root,buffer,0,0,0);
		dynamicfs_delete_node(vfs_lookup(node,"name",0,0,0),1);
		dynamicfs_delete_node(vfs_lookup(node,"exe",0,0,0),0);
		dynamicfs_delete_node(vfs_lookup(node,"stdin",0,0,0),1);
		dynamicfs_delete_node(vfs_lookup(node,"stdout",0,0,0),1);
		dynamicfs_delete_node(vfs_lookup(node,"stderr",0,0,0),1);
		dynamicfs_delete_node(vfs_lookup(node,"threads",0,0,0),1);
		dynamicfs_delete_node(node,0);
	}
}



static u64 _process_self_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	char link[32];
	return dynamicfs_process_simple_read(link,format_string(link,32,"%lu",(THREAD_DATA->header.current_thread?HANDLE_ID_GET_INDEX(THREAD_DATA->process->handle.rb_node.key):0)),offset,buffer,size);
}



void procfs_process_init(void){
	LOG("Creating process subsystem...");
	dynamicfs_create_node(procfs->root,"self",VFS_NODE_TYPE_LINK,NULL,_process_self_read_callback,NULL);
	handle_register_notification_listener(process_handle_type,_listener);
}
