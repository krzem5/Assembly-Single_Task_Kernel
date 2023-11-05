#include <devfs/fs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_pci"



static vfs_node_t* _get_or_alloc_node(vfs_node_t* parent,u8 index){
	char buffer[3];
	SMM_TEMPORARY_STRING name=smm_alloc(buffer,format_string(buffer,3,"%X",index));
	vfs_node_t* out=vfs_node_lookup(parent,name);
	if (!out){
		out=devfs_create_node(parent,buffer,NULL);
	}
	return out;
}



static void _create_device_data_node(vfs_node_t* parent,const char* name,const char* format,...){
	__builtin_va_list va;
	__builtin_va_start(va,format);
	char buffer[16];
	devfs_create_node(parent,name,smm_alloc(buffer,format_string_va(buffer,16,format,&va)));
	__builtin_va_end(va);
}



void devfs_pci_init(void){
	LOG("Creating pci filesystem...");
	vfs_node_t* root=devfs_create_node(devfs->root,"pci",NULL);
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		handle_acquire(handle);
		pci_device_t* device=handle->object;
		vfs_node_t* node=_get_or_alloc_node(_get_or_alloc_node(_get_or_alloc_node(root,device->address.bus),device->address.slot),device->address.func);
		_create_device_data_node(node,"device_id","%X%X",device->device_id>>8,device->device_id);
		_create_device_data_node(node,"vendor_id","%X%X",device->vendor_id>>8,device->vendor_id);
		_create_device_data_node(node,"class","%X",device->class);
		_create_device_data_node(node,"subclass","%X",device->subclass);
		_create_device_data_node(node,"progif","%X",device->progif);
		_create_device_data_node(node,"revision_id","%X",device->revision_id);
		handle_release(handle);
	}
}
