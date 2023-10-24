#ifndef _KERNEL_USB_PIPE_H_
#define _KERNEL_USB_PIPE_H_ 1
#include <kernel/types.h>



struct _USB_DEVICE;



typedef struct _USB_PIPE{
	u8 DATA;
} usb_pipe_t;



usb_pipe_t* usb_pipe_alloc(struct _USB_DEVICE* device,u8 endpoint_address,u8 attributes,u16 max_packet_size);



#endif
