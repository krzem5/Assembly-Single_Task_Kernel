#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/syscall/syscall.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_terminal"



static vfs_node_t* KERNEL_INIT_WRITE _devfs_terminal_root=NULL;
static u64 _devfs_terminal_next_id=0;



static error_t _syscall_create_terminal(KERNEL_USER_POINTER handle_id_t* pipes){
	(void)_devfs_terminal_next_id;
	// /dev/terminal/ter0/{in,out} [temporary]
	// /dev/ter0 -> /dev/terminal/ter0 [returned via callback; marked as temporary if terminal was deleted]
	vfs_node_t* pipe0=pipe_create(NULL,NULL);
	pipe0->flags|=0660<<VFS_NODE_PERMISSION_SHIFT;
	pipe0->uid=THREAD_DATA->process->uid;
	pipe0->gid=THREAD_DATA->process->gid;
	vfs_node_t* pipe1=pipe_create(NULL,NULL);
	pipe1->flags|=0660<<VFS_NODE_PERMISSION_SHIFT;
	pipe1->uid=THREAD_DATA->process->uid;
	pipe1->gid=THREAD_DATA->process->gid;
	pipes[0]=fd_from_node(pipe0,FD_FLAG_READ|FD_FLAG_WRITE);
	pipes[1]=fd_from_node(pipe1,FD_FLAG_READ|FD_FLAG_WRITE);
	vfs_node_unref(pipe0);
	vfs_node_unref(pipe1);
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



