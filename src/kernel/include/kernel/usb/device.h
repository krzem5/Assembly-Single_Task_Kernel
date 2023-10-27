#ifndef _KERNEL_USB_DEVICE_H_
#define _KERNEL_USB_DEVICE_H_ 1
#include <kernel/types.h>
#include <kernel/usb/address.h>
#include <kernel/usb/controller.h>
#include <kernel/usb/pipe.h>



#define USB_DEVICE_SPEED_INVALID 0
#define USB_DEVICE_SPEED_FULL 1
#define USB_DEVICE_SPEED_LOW 2
#define USB_DEVICE_SPEED_HIGH 3
#define USB_DEVICE_SPEED_SUPER 4

#define USB_DEVICE_TYPE_HUB 0
#define USB_DEVICE_TYPE_DEVICE 1



typedef struct _USB_DEVICE_DESCRIPTOR{
	u16 version;
	u8 device_class;
	u8 device_subclass;
	u8 device_protocol;
	u16 max_packet_size;
	u16 vendor;
	u16 product;
	u8 manufacturer_string;
	u8 product_string;
	u8 serial_number_string;
	u8 configuration_count;
} usb_device_descriptor_t;



typedef struct _USB_INTERFACE_DESCRIPTOR{
	struct _USB_INTERFACE_DESCRIPTOR* next;
	u8 index;
	u8 alternate_setting;
	u8 endpoint_count;
	u8 class;
	u8 subclass;
	u8 protocol;
	u8 name_string;
} usb_interface_descriptor_t;



typedef struct _USB_CONFIGURATION_DESCRIPTOR{
	struct _USB_CONFIGURATION_DESCRIPTOR* next;
	u8 value;
	u8 interface_count;
	u8 name_string;
	u8 attributes;
	u8 max_power;
	usb_interface_descriptor_t* interface;
} usb_configuration_descriptor_t;



typedef struct _USB_DEVICE{
	const usb_controller_t* controller;
	struct _USB_DEVICE* parent;
	struct _USB_DEVICE* prev;
	struct _USB_DEVICE* next;
	u8 type;
	u8 speed;
	u8 address;
	u8 port;
	usb_pipe_t* default_pipe;
	usb_device_descriptor_t* device_descriptor;
	usb_configuration_descriptor_t* configuration_descriptor;
	union{
		struct{
			u16 port_count;
			_Bool is_root_hub;
			struct _USB_DEVICE* child;
			usb_address_space_t address_space;
		} hub;
	};
} usb_device_t;



usb_device_t* usb_device_alloc(const usb_controller_t* controller,u8 type,u16 port);



void usb_device_dealloc(usb_device_t* device);



void usb_device_enumerate_children(usb_device_t* hub);



#endif
