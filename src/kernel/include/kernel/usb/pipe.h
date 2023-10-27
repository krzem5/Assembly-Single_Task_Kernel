#ifndef _KERNEL_USB_PIPE_H_
#define _KERNEL_USB_PIPE_H_ 1
#include <kernel/types.h>
#include <kernel/usb/_usb_types.h>
#include <kernel/usb/structures.h>



usb_pipe_t* usb_pipe_alloc(usb_device_t* device,u8 endpoint_address,u8 attributes,u16 max_packet_size);



void usb_pipe_resize(usb_device_t* device,usb_pipe_t* pipe,u16 max_packet_size);



void usb_pipe_transfer_setup(usb_device_t* device,usb_pipe_t* pipe,const usb_raw_control_request_t* request,void* data);



void usb_pipe_transfer_normal(usb_device_t* device,usb_pipe_t* pipe,void* data,u16 length);



#endif
