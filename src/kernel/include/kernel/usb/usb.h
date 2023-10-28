#ifndef _KERNEL_USB_USB_H_
#define _KERNEL_USB_USB_H_ 1
#include <kernel/usb/_usb_types.h>



extern handle_type_t HANDLE_TYPE_USB_DRIVER_DESCRIPTOR;



void usb_register_driver(usb_driver_descriptor_t* driver);



void usb_unregister_driver(usb_driver_descriptor_t* driver);



void usb_device_attach_driver(usb_device_t* device,usb_driver_t* driver);



void usb_device_dettach_driver(usb_device_t* device,usb_driver_t* driver);



#endif
