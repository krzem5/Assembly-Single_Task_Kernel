#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_pci"



void devfs_pci_init(void){
	LOG("Creating pci subsystem...");
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"pci",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		handle_acquire(handle);
		const pci_device_t* device=handle->object;
		char buffer[32];
		format_string(buffer,32,"pci%us%uf%u",device->address.bus,device->address.slot,device->address.func);
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"handle","%lu",handle->rb_node.key);
		dynamicfs_create_data_node(node,"device_id","%X%X",device->device_id>>8,device->device_id);
		dynamicfs_create_data_node(node,"vendor_id","%X%X",device->vendor_id>>8,device->vendor_id);
		dynamicfs_create_data_node(node,"class","%X",device->class);
		dynamicfs_create_data_node(node,"subclass","%X",device->subclass);
		dynamicfs_create_data_node(node,"progif","%X",device->progif);
		dynamicfs_create_data_node(node,"revision_id","%X",device->revision_id);
		dynamicfs_create_link_node(devfs->root,buffer,"pci/%s",buffer);
		handle_release(handle);
	}
}
