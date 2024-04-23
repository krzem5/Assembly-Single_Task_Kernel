#ifndef _KERNEL_USB__USB_TYPES_H_
#define _KERNEL_USB__USB_TYPES_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>
#include <kernel/usb/address_space.h>
#include <kernel/usb/structures.h>



struct _USB_CONTROLLER;
struct _USB_DEVICE;
struct _USB_DRIVER;



typedef void usb_pipe_t;



typedef struct _USB_ENDPOINT_DESCRIPTOR{
	struct _USB_ENDPOINT_DESCRIPTOR* next;
	u8 address;
	u8 attributes;
	u16 max_packet_size;
	u8 interval;
} usb_endpoint_descriptor_t;



typedef struct _USB_INTERFACE_DESCRIPTOR{
	struct _USB_INTERFACE_DESCRIPTOR* next;
	u8 index;
	u8 alternate_setting;
	u8 endpoint_count;
	u8 class;
	u8 subclass;
	u8 protocol;
	u8 name_string;
	usb_endpoint_descriptor_t* endpoint;
	struct _USB_DRIVER* driver;
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



typedef struct _USB_DRIVER_DESCRIPTOR{
	const char* name;
	bool (*load_callback)(struct _USB_DEVICE*,usb_interface_descriptor_t*);
	handle_t handle;
} usb_driver_descriptor_t;



typedef struct _USB_DRIVER{
	usb_driver_descriptor_t* descriptor;
} usb_driver_t;



typedef struct _USB_DEVICE{
	handle_t handle;
	struct _USB_CONTROLLER* controller;
	struct _USB_DEVICE* parent;
	struct _USB_DEVICE* prev;
	struct _USB_DEVICE* next;
	struct _USB_DEVICE* child;
	u8 speed;
	u8 address;
	u8 port;
	usb_pipe_t* default_pipe;
	usb_device_descriptor_t* device_descriptor;
	usb_configuration_descriptor_t* configuration_descriptor;
	usb_configuration_descriptor_t* current_configuration_descriptor;
} usb_device_t;



typedef struct _USB_ROOT_CONTROLLER{
	void* device;
	usb_pipe_t* (*pipe_alloc)(void*,usb_device_t*,u8,u8,u16);
	void (*pipe_resize)(void*,usb_device_t*,usb_pipe_t*,u16);
	void (*pipe_transfer_setup)(void*,usb_device_t*,usb_pipe_t*,const usb_raw_control_request_t*,void*);
	void (*pipe_transfer_normal)(void*,usb_device_t*,usb_pipe_t*,void*,u16);
	usb_address_space_t address_space;
} usb_root_controller_t;



typedef struct _USB_CONTROLLER{
	usb_root_controller_t* root_controller;
	void* device;
	void (*disconnect)(void*,u16);
} usb_controller_t;



#endif
