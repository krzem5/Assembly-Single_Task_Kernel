#ifndef _KERNEL_USB_DEVICE_H_
#define _KERNEL_USB_DEVICE_H_ 1
#include <kernel/types.h>
#include <kernel/usb/_usb_types.h>



#define USB_DEVICE_SPEED_INVALID 0
#define USB_DEVICE_SPEED_FULL 1
#define USB_DEVICE_SPEED_LOW 2
#define USB_DEVICE_SPEED_HIGH 3
#define USB_DEVICE_SPEED_SUPER 4

#define USB_DEVICE_TYPE_HUB 0
#define USB_DEVICE_TYPE_DEVICE 1



usb_device_t* usb_device_alloc(const usb_controller_t* controller,u8 type,u16 port);



void usb_device_enumerate_children(usb_device_t* hub);



#endif
