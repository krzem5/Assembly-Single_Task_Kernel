#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/usb/address.h>
#include <kernel/usb/controller.h>
#include <kernel/usb/device.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "usb_device"



static pmm_counter_descriptor_t _usb_device_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_device");
static omm_allocator_t _usb_device_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_device",sizeof(usb_device_t),8,2,&_usb_device_omm_pmm_counter);



static void _set_device_address(usb_device_t* device){
	usb_address_space_dealloc(&(device->parent->hub.address_space),device->address);
	device->address=usb_address_space_alloc(&(device->parent->hub.address_space));
	LOG("Port: %u, Speed: %u, Address: %X",device->port,device->speed,device->address);
}



usb_device_t* usb_device_alloc(const usb_controller_t* controller,u8 type,u16 port){
	usb_device_t* out=omm_alloc(&_usb_device_allocator);
	out->controller=controller;
	out->parent=NULL;
	out->prev=NULL;
	out->next=NULL;
	out->type=type;
	out->speed=USB_DEVICE_SPEED_HIGH;
	out->address=0;
	out->port=port;
	if (type==USB_DEVICE_TYPE_HUB){
		out->hub.port_count=0;
		out->hub.child=NULL;
		usb_address_space_init(&(out->hub.address_space));
	}
	return out;
}



void usb_device_dealloc(usb_device_t* device){
	omm_dealloc(&_usb_device_allocator,device);
}



void usb_device_enumerate_children(usb_device_t* hub){
	if (hub->type!=USB_DEVICE_TYPE_HUB){
		panic("usb_device_enumerate_children: usb device is not a hub");
	}
	for (u16 i=0;i<hub->hub.port_count;i++){
		if (!hub->controller->detect(hub->controller->device,i)){
			continue;
		}
		u8 speed=hub->controller->reset(hub->controller->device,i);
		if (speed==USB_DEVICE_SPEED_INVALID){
			continue;
		}
		usb_device_t* device=usb_device_alloc(hub->controller,USB_DEVICE_TYPE_DEVICE,i);
		device->parent=hub;
		device->next=hub->hub.child;
		hub->hub.child=device;
		device->speed=speed;
		_set_device_address(device);
	}
}
