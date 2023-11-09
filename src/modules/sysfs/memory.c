#include <sysfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/log/log.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "sysfs_memory"



void sysfs_memory_init(void){
	LOG("Creating memory subsystem...");
	vfs_node_t* root=dynamicfs_create_node(sysfs->root,"memory",VFS_NODE_TYPE_DIRECTORY,NULL);
	(void)root;
}
