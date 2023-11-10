#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/log/log.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_virtual_file"



static u64 _zero_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	memset(buffer,0,size);
	return size;
}



static u64 _one_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	memset(buffer,0xff,size);
	return size;
}



static u64 _random_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	random_generate(buffer,size);
	return size;
}



static void _stdin_callback(vfs_node_t* node){
	while (1){
		u8 byte;
		vfs_node_write(node,0,&byte,serial_recv(&byte,1,0));
		return;
	}
}



static void _stdout_callback(vfs_node_t* node){
	while (1){
		u8 byte;
		serial_send(&byte,vfs_node_read(node,0,&byte,1));
	}
}



static void _create_pipe(const char* name,void* callback){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	vfs_node_t* node=pipe_create(devfs->root,name_string);
	thread_t* thread=thread_new_kernel_thread(process_kernel,callback,0x10000,1,node);
	thread->priority=SCHEDULER_PRIORITY_HIGH;
	scheduler_enqueue_thread(thread);
}



void devfs_virtual_file_init(void){
	LOG("Creating virtual file subsystem...");
	dynamicfs_create_node(devfs->root,"zero",VFS_NODE_TYPE_FILE,NULL,_zero_read_callback,NULL);
	dynamicfs_create_node(devfs->root,"one",VFS_NODE_TYPE_FILE,NULL,_one_read_callback,NULL);
	dynamicfs_create_node(devfs->root,"random",VFS_NODE_TYPE_FILE,NULL,_random_read_callback,NULL);
	_create_pipe("stdin",_stdin_callback);
	_create_pipe("stdout",_stdout_callback);
}
