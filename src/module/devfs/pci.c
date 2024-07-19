#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/pci/pci.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_pci"



MODULE_POSTINIT(){
	LOG("Creating pci subsystem...");
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"pci",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	HANDLE_FOREACH(pci_device_handle_type){
		const pci_device_t* device=KERNEL_CONTAINEROF(handle,const pci_device_t,handle);
		char buffer[32];
		format_string(buffer,32,"pci%ub%us%uf%u",device->segment_group,device->bus,device->slot,device->func);
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		vfs_node_unref(dynamicfs_create_data_node(node,"id","%lu",HANDLE_ID_GET_INDEX(handle->rb_node.key)));
		vfs_node_unref(dynamicfs_create_data_node(node,"device_id","%X%X",device->device_id>>8,device->device_id));
		vfs_node_unref(dynamicfs_create_data_node(node,"vendor_id","%X%X",device->vendor_id>>8,device->vendor_id));
		vfs_node_unref(dynamicfs_create_data_node(node,"class","%X",device->class));
		vfs_node_unref(dynamicfs_create_data_node(node,"subclass","%X",device->subclass));
		vfs_node_unref(dynamicfs_create_data_node(node,"progif","%X",device->progif));
		vfs_node_unref(dynamicfs_create_data_node(node,"revision_id","%X",device->revision_id));
		vfs_node_unref(node);
		vfs_node_unref(dynamicfs_create_link_node(devfs->root,buffer,"pci/%s",buffer));
	}
	vfs_node_unref(root);
}
