#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/serial/serial.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_serial"



#define SERIAL_BUFFER_SIZE 256



static void _stdin_callback(vfs_node_t* node,serial_port_t* port){
	u8 buffer[1];
	while (1){
		vfs_node_write(node,0,buffer,serial_recv(port,buffer,1),0);
	}
}



static void _stdout_callback(vfs_node_t* node,serial_port_t* port){
	u8 buffer[SERIAL_BUFFER_SIZE];
	while (1){
		serial_send(port,buffer,vfs_node_read(node,0,buffer,SERIAL_BUFFER_SIZE,0));
	}
}



static vfs_node_t* _create_pipe(vfs_node_t* parent,u8 port_index,const char* name,void* callback,serial_port_t* port){
	SMM_TEMPORARY_STRING name_string=smm_alloc(name,0);
	vfs_node_t* node=pipe_create(parent,name_string);
	char buffer[64];
	format_string(buffer,64,"devfs-serial-pipe-ser%u%s",port_index,name);
	thread_create_kernel_thread(NULL,buffer,callback,0x200000,2,node,port)->priority=SCHEDULER_PRIORITY_REALTIME;
	return node;
}



void devfs_serial_init(void){
	LOG("Creating serial subsystem...");
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"serial",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u8 i=0;i<SERIAL_PORT_COUNT;i++){
		serial_port_t* port=serial_ports+i;
		if (!port->io_port){
			continue;
		}
		char buffer[8];
		format_string(buffer,8,"ser%u",i);
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"id","%u",i);
		_create_pipe(node,i,"in",_stdin_callback,port)->flags|=0444<<VFS_NODE_PERMISSION_SHIFT;
		_create_pipe(node,i,"out",_stdout_callback,port)->flags|=0222<<VFS_NODE_PERMISSION_SHIFT;
		dynamicfs_create_link_node(devfs->root,buffer,"serial/%s",buffer);
	}
	if (serial_default_port){
		dynamicfs_create_link_node(devfs->root,"ser","ser%u",serial_default_port-serial_ports);
	}
}
