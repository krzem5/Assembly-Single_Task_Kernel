#include <dynamicfs/dynamicfs.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_kernel"



void sysfs_kernel_init(void){
	LOG("Creating kernel subsystem...");
	vfs_node_t* root=dynamicfs_create_node(sysfs->root,"kernel",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_create_data_node(root,"version","%lx",kernel_get_version());
}
