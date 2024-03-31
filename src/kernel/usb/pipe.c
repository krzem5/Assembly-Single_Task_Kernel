#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/structures.h>



KERNEL_PUBLIC usb_pipe_t* usb_pipe_alloc(usb_device_t* device,u8 endpoint_address,u8 attributes,u16 max_packet_size){
	return device->controller->root_controller->pipe_alloc(device->controller->root_controller->device,device,endpoint_address,attributes,max_packet_size);
}



KERNEL_PUBLIC void usb_pipe_resize(usb_device_t* device,usb_pipe_t* pipe,u16 max_packet_size){
	device->controller->root_controller->pipe_resize(device->controller->root_controller->device,device,pipe,max_packet_size);
}



KERNEL_PUBLIC void usb_pipe_transfer_setup(usb_device_t* device,usb_pipe_t* pipe,const usb_raw_control_request_t* request,void* data){
	device->controller->root_controller->pipe_transfer_setup(device->controller->root_controller->device,device,pipe,request,data);
}



KERNEL_PUBLIC void usb_pipe_transfer_normal(usb_device_t* device,usb_pipe_t* pipe,void* data,u16 length){
	device->controller->root_controller->pipe_transfer_normal(device->controller->root_controller->device,device,pipe,data,length);
}
