#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_memory"



MODULE_POSTINIT(){
	LOG("Creating memory subsystem...");
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"memory",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		char buffer[16];
		format_string(buffer,16,"mem%u",i);
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"address","%lu",(kernel_data.mmap+i)->base);
		dynamicfs_create_data_node(node,"length","%lu",(kernel_data.mmap+i)->length);
		dynamicfs_create_link_node(devfs->root,buffer,"memory/%s",buffer);
	}
}
