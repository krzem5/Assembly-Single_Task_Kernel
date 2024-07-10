#include <dynamicfs/dynamicfs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_memory"



MODULE_POSTINIT(){
	LOG("Creating memory subsystem...");
	vfs_node_t* root=dynamicfs_create_node(sysfs->root,"memory",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(root);
	vfs_node_t* node=dynamicfs_create_node(root,"lb",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(node);
	vfs_node_unref(dynamicfs_set_root_only(dynamicfs_create_node(node,"hit",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->hit_count)))));
	vfs_node_unref(dynamicfs_set_root_only(dynamicfs_create_node(node,"miss",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->miss_count)))));
	vfs_node_unref(dynamicfs_set_root_only(dynamicfs_create_node(node,"expensive_miss",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->miss_locked_count)))));
	vfs_node_unref(dynamicfs_set_root_only(dynamicfs_create_node(node,"page_clean",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->clean_count)))));
	vfs_node_unref(dynamicfs_set_root_only(dynamicfs_create_node(node,"page_dirty",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->dirty_count)))));
	vfs_node_unref(node);
	vfs_node_unref(root);
}
