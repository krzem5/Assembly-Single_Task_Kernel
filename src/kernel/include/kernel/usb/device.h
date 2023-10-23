#ifndef _KERNEL_USB_DEVICE_H_
#define _KERNEL_USB_DEVICE_H_ 1
#include <kernel/types.h>
#include <kernel/usb/address.h>
#include <kernel/usb/controller.h>



#define USB_DEVICE_SPEED_INVALID 0
#define USB_DEVICE_SPEED_FULL 1
#define USB_DEVICE_SPEED_LOW 2
#define USB_DEVICE_SPEED_HIGH 3
#define USB_DEVICE_SPEED_SUPER 4

#define USB_DEVICE_TYPE_HUB 0
#define USB_DEVICE_TYPE_DEVICE 1



typedef struct _USB_DEVICE{
	const usb_controller_t* controller;
	struct _USB_DEVICE* parent;
	struct _USB_DEVICE* prev;
	struct _USB_DEVICE* next;
	u8 type;
	u8 speed;
	u8 address;
	u16 port;
	union{
		struct{
			u16 port_count;
			struct _USB_DEVICE* child;
			usb_address_space_t address_space;
		} hub;
	};
} usb_device_t;



usb_device_t* usb_device_alloc(const usb_controller_t* controller,u8 type,u16 port);



void usb_device_dealloc(usb_device_t* device);



void usb_device_enumerate_children(usb_device_t* hub);



#endif
