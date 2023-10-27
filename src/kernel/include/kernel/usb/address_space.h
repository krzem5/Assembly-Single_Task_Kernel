#ifndef _KERNEL_USB_ADDRESS_SPACE_H_
#define _KERNEL_USB_ADDRESS_SPACE_H_ 1
#include <kernel/types.h>



typedef struct _USB_ADDRESS_SPACE{
	u64 data[2];
} usb_address_space_t;



void usb_address_space_init(usb_address_space_t* address_space);



u8 usb_address_space_alloc(usb_address_space_t* address_space);



void usb_address_space_dealloc(usb_address_space_t* address_space,u8 address);


#endif
