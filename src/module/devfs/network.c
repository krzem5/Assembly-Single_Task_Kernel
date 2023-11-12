#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/network/layer1.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_network"



void devfs_network_init(void){
	LOG("Creating network subsystem...");
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"network",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_create_data_node(root,"address","%X:%X:%X:%X:%X:%X",network_layer1_mac_address[0],network_layer1_mac_address[1],network_layer1_mac_address[2],network_layer1_mac_address[3],network_layer1_mac_address[4],network_layer1_mac_address[5]);
}
