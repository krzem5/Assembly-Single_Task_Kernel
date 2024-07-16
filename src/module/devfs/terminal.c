#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/socket/socket.h>
#include <kernel/syscall/syscall.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_terminal"



static vfs_node_t* KERNEL_INIT_WRITE _devfs_terminal_root=NULL;
static u64 _devfs_terminal_next_id=0;



static u64 _link_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!ctx){
		return 0;
	}
	vfs_node_t* node=ctx;
	vfs_node_t* target=vfs_lookup(_devfs_terminal_root,node->name->data,0,0,0);
	if (!target){
		node->flags|=VFS_NODE_FLAG_TEMPORARY;
		return 0;
	}
	vfs_node_unref(target);
	char link[64];
	return dynamicfs_process_simple_read(link,format_string(link,64,"terminal/%s",node->name->data),offset,buffer,size);
}



static error_t _syscall_create_terminal(KERNEL_USER_POINTER handle_id_t* pipes){
	if (syscall_get_user_pointer_max_length((void*)pipes)<3*sizeof(handle_id_t)){
		return ERROR_INVALID_ARGUMENT(0);
	}
	u64 id=__atomic_fetch_add(&_devfs_terminal_next_id,1,__ATOMIC_SEQ_CST);
	char buffer[64];
	format_string(buffer,sizeof(buffer),"ter%lu",id);
	vfs_node_t* node=dynamicfs_create_node(_devfs_terminal_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	node->flags|=VFS_NODE_FLAG_TEMPORARY;
	string_t* name_string=smm_alloc("in",0);
	vfs_node_t* input_pipe=pipe_create(node,name_string);
	smm_dealloc(name_string);
	input_pipe->flags|=(0660<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_TEMPORARY;
	input_pipe->uid=THREAD_DATA->process->uid;
	input_pipe->gid=THREAD_DATA->process->gid;
	pipes[0]=fd_from_node(input_pipe,FD_FLAG_READ|FD_FLAG_WRITE);
	vfs_node_unref(input_pipe);
	name_string=smm_alloc("out",0);
	vfs_node_t* output_pipe=pipe_create(node,name_string);
	smm_dealloc(name_string);
	output_pipe->flags|=(0660<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_TEMPORARY;
	output_pipe->uid=THREAD_DATA->process->uid;
	output_pipe->gid=THREAD_DATA->process->gid;
	pipes[1]=fd_from_node(output_pipe,FD_FLAG_READ|FD_FLAG_WRITE);
	vfs_node_unref(output_pipe);
	vfs_node_t* ctrl_socket=socket_create(SOCKET_DOMAIN_UNIX,SOCKET_TYPE_DGRAM,SOCKET_PROTOCOL_NONE);
	ctrl_socket->flags|=(0660<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_TEMPORARY;
	ctrl_socket->uid=THREAD_DATA->process->uid;
	ctrl_socket->gid=THREAD_DATA->process->gid;
	socket_move_direct(ctrl_socket,node,"ctrl");
	pipes[2]=fd_from_node(ctrl_socket,FD_FLAG_READ|FD_FLAG_WRITE);
	vfs_node_unref(ctrl_socket);
	vfs_node_unref(node);
	vfs_node_t* link_node=dynamicfs_create_node(devfs->root,buffer,VFS_NODE_TYPE_LINK,NULL,_link_read_callback,NULL);
	dynamicfs_change_ctx(link_node,link_node);
	vfs_node_unref(link_node);
	return ERROR_OK;
}



static syscall_callback_t const _devfs_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_create_terminal,
};



MODULE_POSTINIT(){
	LOG("Creating terminal subsystem...");
	_devfs_terminal_root=dynamicfs_create_node(devfs->root,"terminal",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	syscall_create_table("devfs",_devfs_syscall_functions,sizeof(_devfs_syscall_functions)/sizeof(syscall_callback_t));
}



