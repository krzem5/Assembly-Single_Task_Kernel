#ifndef _KERNEL_USB_USB_H_
#define _KERNEL_USB_USB_H_ 1
#include <kernel/usb/_usb_types.h>



extern handle_type_t usb_driver_descriptor_handle_type;



void usb_register_driver(usb_driver_descriptor_t* driver);



void usb_unregister_driver(usb_driver_descriptor_t* driver);



#endif
