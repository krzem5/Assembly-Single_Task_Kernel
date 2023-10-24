#ifndef _KERNEL_USB_CONTROLLER_H_
#define _KERNEL_USB_CONTROLLER_H_ 1
#include <kernel/types.h>
#include <kernel/usb/pipe.h>



struct _USB_DEVICE;



typedef struct _USB_ROOT_CONTROLLER{
	void* device;
	usb_pipe_t* (*pipe_update)(void*,struct _USB_DEVICE*,u8,u8,u16);
	void (*pipe_transfer_setup)(void*,struct _USB_DEVICE*,usb_pipe_t*,const usb_control_request_t*,void*);
	void (*pipe_transfer_normal)(void*,struct _USB_DEVICE*,usb_pipe_t*,void*,u16);
} usb_root_controller_t;



typedef struct _USB_CONTROLLER{
	const usb_root_controller_t* root_controller;
	void* device;
	_Bool (*detect)(void*,u16);
	u8 (*reset)(void*,u16);
	void (*disconnect)(void*,u16);
} usb_controller_t;



usb_root_controller_t* usb_root_controller_alloc(void);



usb_controller_t* usb_controller_alloc(const usb_root_controller_t* root_controller);



void usb_root_controller_dealloc(usb_root_controller_t* root_controller);



void usb_controller_dealloc(usb_controller_t* controller);


#endif
