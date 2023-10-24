#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/structures.h>
#include <kernel/util/util.h>



usb_pipe_t* usb_pipe_alloc(usb_device_t* device,u8 endpoint_address,u8 attributes,u16 max_packet_size){
	panic("usb_pipe_alloc");
}



void usb_pipe_transfer_setup(usb_pipe_t* pipe,const usb_control_request_t* request,void* data){
	panic("usb_pipe_transfer_setup");
}



void usb_pipe_transfer_normal(usb_pipe_t* pipe,void* data,u16 length){
	panic("usb_pipe_transfer_normal");
}
