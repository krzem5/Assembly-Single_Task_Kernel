#include <devfs/fs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_pci"



void devfs_pci_init(void){
	LOG("Creating pci subsystem...");
	vfs_node_t* root=devfs_create_node(devfs->root,"pci",NULL);
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		handle_acquire(handle);
		pci_device_t* device=handle->object;
		char buffer[32];
		format_string(buffer,32,"b%us%uf%u",device->address.bus,device->address.slot,device->address.func);
		vfs_node_t* node=devfs_create_node(root,buffer,NULL);
		devfs_create_data_node(node,"device_id","%X%X",device->device_id>>8,device->device_id);
		devfs_create_data_node(node,"vendor_id","%X%X",device->vendor_id>>8,device->vendor_id);
		devfs_create_data_node(node,"class","%X",device->class);
		devfs_create_data_node(node,"subclass","%X",device->subclass);
		devfs_create_data_node(node,"progif","%X",device->progif);
		devfs_create_data_node(node,"revision_id","%X",device->revision_id);
		handle_release(handle);
	}
}
