#ifndef _KERNEL_USB_CONTROLLER_H_
#define _KERNEL_USB_CONTROLLER_H_ 1
#include <kernel/usb/_usb_types.h>



usb_root_controller_t* usb_root_controller_alloc(void);



usb_controller_t* usb_controller_alloc(usb_root_controller_t* root_controller);



void usb_root_controller_dealloc(usb_root_controller_t* root_controller);



void usb_controller_dealloc(usb_controller_t* controller);



#endif
