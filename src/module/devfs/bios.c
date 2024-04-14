#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/bios/bios.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_bios"



MODULE_POSTINIT(){
	LOG("Creating bios subsystem...");
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"bios",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_create_node(root,"bios_vendor",VFS_NODE_TYPE_FILE,bios_data.bios_vendor,NULL,NULL);
	dynamicfs_create_node(root,"bios_version",VFS_NODE_TYPE_FILE,bios_data.bios_version,NULL,NULL);
	dynamicfs_create_node(root,"manufacturer",VFS_NODE_TYPE_FILE,bios_data.manufacturer,NULL,NULL);
	dynamicfs_create_node(root,"product",VFS_NODE_TYPE_FILE,bios_data.product,NULL,NULL);
	dynamicfs_create_node(root,"version",VFS_NODE_TYPE_FILE,bios_data.version,NULL,NULL);
	dynamicfs_create_node(root,"serial_number",VFS_NODE_TYPE_FILE,bios_data.serial_number,NULL,NULL);
	dynamicfs_create_node(root,"uuid",VFS_NODE_TYPE_FILE,bios_data.uuid_str,NULL,NULL);
	dynamicfs_create_node(root,"wakeup_type",VFS_NODE_TYPE_FILE,bios_data.wakeup_type_str,NULL,NULL);
}
