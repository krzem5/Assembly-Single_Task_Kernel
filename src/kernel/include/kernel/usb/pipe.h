#ifndef _KERNEL_USB_PIPE_H_
#define _KERNEL_USB_PIPE_H_ 1
#include <kernel/types.h>
#include <kernel/usb/structures.h>



struct _USB_DEVICE;



typedef void usb_pipe_t;



usb_pipe_t* usb_pipe_alloc(struct _USB_DEVICE* device,u8 endpoint_address,u8 attributes,u16 max_packet_size);



void usb_pipe_transfer_setup(struct _USB_DEVICE* device,usb_pipe_t* pipe,const usb_control_request_t* request,void* data);



void usb_pipe_transfer_normal(struct _USB_DEVICE* device,usb_pipe_t* pipe,void* data,u16 length);



#endif
