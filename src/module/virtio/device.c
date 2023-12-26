#include <kernel/apic/ioapic.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/pci/pci.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/device.h>
#include <virtio/registers.h>
#define KERNEL_LOG_NAME "virtio"



static pmm_counter_descriptor_t* _virtio_driver_pmm_counter=NULL;
static omm_allocator_t* _virtio_device_allocator=NULL;
static omm_allocator_t* _virtio_device_driver_node_allocator=NULL;
static rb_tree_t _virtio_device_driver_tree;
static spinlock_t _virtio_device_driver_tree_lock;

static handle_type_t _virtio_device_handle_type=0;



static void _virtio_init_device(pci_device_t* device){
	if (device->device_id<0x1000||device->device_id>0x107f||device->vendor_id!=0x1af4){
		return;
	}
	LOG("Attached virtio driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	pci_device_enable_bus_mastering(device);
	pci_device_enable_memory_access(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,0,&pci_bar)){
		return;
	}
	virtio_device_t* virtio_device=omm_alloc(_virtio_device_allocator);
	handle_new(virtio_device,_virtio_device_handle_type,&(virtio_device->handle));
	virtio_device->type=pci_device_read_data(device,0x2c)>>16;
	virtio_device->port=pci_bar.address;
	spinlock_init(&(virtio_device->lock));
	INFO("Device type: %u, Device port: %x",virtio_device->type,virtio_device->port);
	io_port_out8(virtio_device->port+VIRTIO_REGISTER_DEVICE_STATUS,0x00);
	io_port_out8(virtio_device->port+VIRTIO_REGISTER_DEVICE_STATUS,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE);
	handle_finish_setup(&(virtio_device->handle));
	// // Lookup driver base on virtio_device->type
	// io_port_out8(virtio_device->port+VIRTIO_REGISTER_DEVICE_STATUS,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER);
	// // negotiate features
	// io_port_out32(virtio_device->port+VIRTIO_REGISTER_GUEST_FEATURES,io_port_in32(virtio_device->port+VIRTIO_REGISTER_DEVICE_FEATURES));
	// io_port_out8(virtio_device->port+VIRTIO_REGISTER_DEVICE_STATUS,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
	// if (!(io_port_in8(virtio_device->port+VIRTIO_REGISTER_DEVICE_STATUS)&VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK)){
	// 	io_port_out8(virtio_device->port+VIRTIO_REGISTER_DEVICE_STATUS,VIRTIO_DEVICE_STATUS_FLAG_FAILED);
	// 	ERROR("Failed to initialize virtio device");
	// 	omm_dealloc(_virtio_device_allocator,virtio_device);
	// 	return;
	// }
	// WARN("%u %x",device->interrupt_line,virtio_device->port);
	// // VIRTIO_DEVICE_STATUS_FLAG_DRIVER_READY
	// panic("A");
}



KERNEL_PUBLIC _Bool virtio_register_device_driver(const virtio_device_driver_t* driver){
	return 0;
}



KERNEL_PUBLIC _Bool virtio_unregister_device_driver(const virtio_device_driver_t* driver){
	return 0;
}



void virtio_locate_devices(void){
	LOG("Initializing VirtIO driver...");
	_virtio_driver_pmm_counter=pmm_alloc_counter("virtio");
	_virtio_device_allocator=omm_init("virtio_device",sizeof(virtio_device_t),8,1,pmm_alloc_counter("omm_virtio_device"));
	spinlock_init(&(_virtio_device_allocator->lock));
	_virtio_device_driver_node_allocator=omm_init("virtio_device_driver_node",sizeof(virtio_device_driver_node_t),8,1,pmm_alloc_counter("omm_virtio_device_driver_node"));
	spinlock_init(&(_virtio_device_driver_node_allocator->lock));
	_virtio_device_handle_type=handle_alloc("virtio_device",NULL);
	rb_tree_init(&_virtio_device_driver_tree);
	spinlock_init(&_virtio_device_driver_tree_lock);
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_virtio_init_device(device);
	}
}
