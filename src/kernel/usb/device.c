#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/usb/address_space.h>
#include <kernel/usb/controller.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/structures.h>
#include <kernel/usb/usb.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "usb_device"



static pmm_counter_descriptor_t* _usb_buffer_pmm_counter=NULL;
static omm_allocator_t* _usb_device_allocator=NULL;
static omm_allocator_t* _usb_device_descriptor_allocator=NULL;
static omm_allocator_t* _usb_configuration_descriptor_allocator=NULL;
static omm_allocator_t* _usb_interface_descriptor_allocator=NULL;
static omm_allocator_t* _usb_endpoint_descriptor_allocator=NULL;

KERNEL_PUBLIC handle_type_t usb_device_handle_type=0;



static KERNEL_INLINE u16 _speed_to_packet_size(u8 speed){
	switch (speed){
		case USB_DEVICE_SPEED_FULL:
			return 8;
		case USB_DEVICE_SPEED_LOW:
			return 8;
		case USB_DEVICE_SPEED_HIGH:
			return 64;
		case USB_DEVICE_SPEED_SUPER:
			return 512;
	}
	panic("_speed_to_packet_size: invalid USB speed");
}



static void _set_device_address(usb_device_t* device){
	usb_address_space_dealloc(&(device->controller->root_controller->address_space),device->address);
	device->address=usb_address_space_alloc(&(device->controller->root_controller->address_space));
	device->default_pipe=usb_pipe_alloc(device,0,USB_ENDPOINT_XFER_CONTROL,_speed_to_packet_size(device->speed));
	usb_raw_control_request_t request={
		USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_DEVICE,
		USB_REQ_SET_ADDRESS,
		device->address,
		0,
		0
	};
	usb_pipe_transfer_setup(device,device->default_pipe,&request,NULL);
}



static void _load_configuration_descriptor(usb_device_t* device,void* buffer,u8 index){
	usb_raw_control_request_t request={
		USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE,
		USB_REQ_GET_DESCRIPTOR,
		USB_DT_CONFIG<<8,
		index,
		sizeof(usb_raw_configuration_descriptor_t)
	};
	usb_pipe_transfer_setup(device,device->default_pipe,&request,buffer);
	const usb_raw_configuration_descriptor_t* raw_configuration_descriptor=buffer;
	request.wLength=raw_configuration_descriptor->wTotalLength;
	usb_pipe_transfer_setup(device,device->default_pipe,&request,buffer);
	usb_configuration_descriptor_t* configuration_descriptor=omm_alloc(_usb_configuration_descriptor_allocator);
	configuration_descriptor->next=device->configuration_descriptor;
	configuration_descriptor->value=raw_configuration_descriptor->bConfigurationValue;
	configuration_descriptor->interface_count=raw_configuration_descriptor->bNumInterfaces;
	configuration_descriptor->name_string=raw_configuration_descriptor->iConfiguration;
	configuration_descriptor->attributes=raw_configuration_descriptor->bmAttributes;
	configuration_descriptor->max_power=raw_configuration_descriptor->bMaxPower;
	configuration_descriptor->interface=NULL;
	device->configuration_descriptor=configuration_descriptor;
	for (u16 i=0;i<raw_configuration_descriptor->wTotalLength-raw_configuration_descriptor->bLength;i+=raw_configuration_descriptor->extra_data[i]){
		switch (raw_configuration_descriptor->extra_data[i+1]){
			case USB_DT_INTERFACE:
				const usb_raw_interface_descriptor_t* raw_interface_descriptor=(const void*)(raw_configuration_descriptor->extra_data+i);
				usb_interface_descriptor_t* interface_descriptor=omm_alloc(_usb_interface_descriptor_allocator);
				interface_descriptor->next=configuration_descriptor->interface;
				interface_descriptor->index=raw_interface_descriptor->bInterfaceNumber;
				interface_descriptor->alternate_setting=raw_interface_descriptor->bAlternateSetting;
				interface_descriptor->endpoint_count=raw_interface_descriptor->bNumEndpoints;
				interface_descriptor->class=raw_interface_descriptor->bInterfaceClass;
				interface_descriptor->subclass=raw_interface_descriptor->bInterfaceSubClass;
				interface_descriptor->protocol=raw_interface_descriptor->bInterfaceProtocol;
				interface_descriptor->name_string=raw_interface_descriptor->iInterface;
				interface_descriptor->endpoint=NULL;
				interface_descriptor->driver=NULL;
				configuration_descriptor->interface=interface_descriptor;
				break;
			case USB_DT_ENDPOINT:
				if (!configuration_descriptor->interface){
					WARN("USB_DT_ENDPOINT before USB_DT_INTERFACE");
					break;
				}
				const usb_raw_endpoint_descriptor_t* raw_endpoint_descriptor=(const void*)(raw_configuration_descriptor->extra_data+i);
				usb_endpoint_descriptor_t* endpoint_descriptor=omm_alloc(_usb_endpoint_descriptor_allocator);
				endpoint_descriptor->next=configuration_descriptor->interface->endpoint;
				endpoint_descriptor->address=raw_endpoint_descriptor->bEndpointAddress;
				endpoint_descriptor->attributes=raw_endpoint_descriptor->bmAttributes;
				endpoint_descriptor->max_packet_size=raw_endpoint_descriptor->wMaxPacketSize;
				endpoint_descriptor->interval=raw_endpoint_descriptor->bInterval;
				configuration_descriptor->interface->endpoint=endpoint_descriptor;
				break;
			case USB_DT_ENDPOINT_COMPANION:
				break;
			default:
				WARN("Unknown USB descriptor type: %X",raw_configuration_descriptor->extra_data[i+1]);
				break;
		}
	}
}



static void _configure_device(usb_device_t* device){
	usb_raw_control_request_t request={
		USB_DIR_IN|USB_TYPE_STANDARD|USB_RECIP_DEVICE,
		USB_REQ_GET_DESCRIPTOR,
		USB_DT_DEVICE<<8,
		0,
		8
	};
	usb_raw_device_descriptor_t descriptor;
	usb_pipe_transfer_setup(device,device->default_pipe,&request,&descriptor);
	request.wLength=descriptor.bLength;
	usb_pipe_transfer_setup(device,device->default_pipe,&request,&descriptor);
	device->device_descriptor=omm_alloc(_usb_device_descriptor_allocator);
	device->device_descriptor->version=descriptor.bcdUSB;
	device->device_descriptor->device_class=descriptor.bDeviceClass;
	device->device_descriptor->device_subclass=descriptor.bDeviceSubClass;
	device->device_descriptor->device_protocol=descriptor.bDeviceProtocol;
	device->device_descriptor->max_packet_size=(descriptor.bcdUSB>=0x300?1<<descriptor.bMaxPacketSize0:descriptor.bMaxPacketSize0);
	device->device_descriptor->vendor=descriptor.idVendor;
	device->device_descriptor->product=descriptor.idProduct;
	device->device_descriptor->manufacturer_string=descriptor.iManufacturer;
	device->device_descriptor->product_string=descriptor.iProduct;
	device->device_descriptor->serial_number_string=descriptor.iSerialNumber;
	device->device_descriptor->configuration_count=descriptor.bNumConfigurations;
	usb_pipe_resize(device,device->default_pipe,device->device_descriptor->max_packet_size);
	device->configuration_descriptor=NULL;
	device->current_configuration_descriptor=NULL;
	void* buffer=(void*)(pmm_alloc(1,_usb_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u8 i=descriptor.bNumConfigurations;i;){
		i--;
		_load_configuration_descriptor(device,buffer,i);
	}
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_usb_buffer_pmm_counter);
	if (device->configuration_descriptor){
		usb_device_set_configuration(device,device->configuration_descriptor->value);
	}
}



KERNEL_INIT(){
	LOG("Initializing USB devices...");
	_usb_buffer_pmm_counter=pmm_alloc_counter("kernel.usb.buffer");
	_usb_device_allocator=omm_init("kernel.usb.device",sizeof(usb_device_t),8,2);
	rwlock_init(&(_usb_device_allocator->lock));
	_usb_device_descriptor_allocator=omm_init("kernel.usb.device_descriptor",sizeof(usb_device_descriptor_t),8,2);
	rwlock_init(&(_usb_device_descriptor_allocator->lock));
	_usb_configuration_descriptor_allocator=omm_init("kernel.usb.configuration_descriptor",sizeof(usb_configuration_descriptor_t),8,4);
	rwlock_init(&(_usb_configuration_descriptor_allocator->lock));
	_usb_interface_descriptor_allocator=omm_init("kernel.usb.interface_descriptor",sizeof(usb_interface_descriptor_t),8,4);
	rwlock_init(&(_usb_interface_descriptor_allocator->lock));
	_usb_endpoint_descriptor_allocator=omm_init("kernel.usb.endpoint_descriptor",sizeof(usb_endpoint_descriptor_t),8,4);
	rwlock_init(&(_usb_endpoint_descriptor_allocator->lock));
	usb_device_handle_type=handle_alloc("kernel.usb.device",NULL);
}



KERNEL_PUBLIC usb_device_t* usb_device_alloc(usb_controller_t* controller,usb_device_t* parent,u16 port,u8 speed){
	usb_device_t* out=omm_alloc(_usb_device_allocator);
	handle_new(usb_device_handle_type,&(out->handle));
	out->controller=controller;
	out->parent=parent;
	out->prev=NULL;
	out->next=NULL;
	out->child=NULL;
	out->speed=speed;
	out->address=0;
	out->port=port;
	out->default_pipe=NULL;
	out->device_descriptor=NULL;
	out->configuration_descriptor=NULL;
	if (!parent){
		return out;
	}
	out->next=parent->child;
	parent->child=out;
	_set_device_address(out);
	LOG("Port: %u, Speed: %u, Address: %X",out->port,out->speed,out->address);
	_configure_device(out);
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC bool usb_device_set_configuration(usb_device_t* device,u8 value){
	usb_configuration_descriptor_t* configuration_descriptor=device->configuration_descriptor;
	for (;configuration_descriptor&&configuration_descriptor->value!=value;configuration_descriptor=configuration_descriptor->next);
	if (!configuration_descriptor){
		return 0;
	}
	if (device->current_configuration_descriptor){
		panic("usb_device_set_configuration: dealloc device->current_configuration_descriptor");
	}
	device->current_configuration_descriptor=configuration_descriptor;
	usb_raw_control_request_t request={
		USB_DIR_OUT|USB_TYPE_STANDARD|USB_RECIP_DEVICE,
		USB_REQ_SET_CONFIGURATION,
		value,
		0,
		0
	};
	usb_pipe_transfer_setup(device,device->default_pipe,&request,NULL);
	for (usb_interface_descriptor_t* interface_descriptor=configuration_descriptor->interface;interface_descriptor;interface_descriptor=interface_descriptor->next){
		HANDLE_FOREACH(usb_driver_descriptor_handle_type){
			handle_acquire(handle);
			usb_driver_descriptor_t* descriptor=KERNEL_CONTAINEROF(handle,usb_driver_descriptor_t,handle);
			if (!descriptor->load_callback(device,interface_descriptor)){
				handle_release(handle);
				continue;
			}
			INFO("Attached driver '%s' to interface of type %X:%X:%X",descriptor->name,interface_descriptor->class,interface_descriptor->subclass,interface_descriptor->protocol);
			break;
		}
	}
	return 1;
}
