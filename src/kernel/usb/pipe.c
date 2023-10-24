#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/structures.h>
#include <kernel/util/util.h>



usb_pipe_t* usb_pipe_alloc(usb_device_t* device,u8 endpoint_address,u8 attributes,u16 max_packet_size){
	return device->controller->root_controller->pipe_alloc(device->controller->root_controller->device,device,endpoint_address,attributes,max_packet_size);
}



void usb_pipe_transfer_setup(usb_device_t* device,usb_pipe_t* pipe,const usb_control_request_t* request,void* data){
	device->controller->root_controller->pipe_transfer_setup(device->controller->root_controller->device,device,pipe,request,data);
}



void usb_pipe_transfer_normal(usb_device_t* device,usb_pipe_t* pipe,void* data,u16 length){
	device->controller->root_controller->pipe_transfer_normal(device->controller->root_controller->device,device,pipe,data,length);
}
