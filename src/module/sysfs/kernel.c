#include <dynamicfs/dynamicfs.h>
#include <kernel/kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_kernel"



MODULE_POSTINIT(){
	LOG("Creating kernel subsystem...");
	vfs_node_t* root=dynamicfs_create_node(sysfs->root,"kernel",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	vfs_node_unref(dynamicfs_create_data_node(root,"version","%lx",kernel_get_version()));
	vfs_node_unref(dynamicfs_create_data_node(root,"build","%s",kernel_get_build_name()));
	vfs_node_unref(root);
}
