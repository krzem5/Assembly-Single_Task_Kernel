#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/usb/device.h>
#include <kernel/usb/usb.h>
#define KERNEL_LOG_NAME "usb"



handle_type_t usb_driver_descriptor_handle_type=0;



void usb_register_driver(usb_driver_descriptor_t* driver){
	LOG("Registering USB driver '%s'...",driver->name);
	if (!usb_driver_descriptor_handle_type){
		usb_driver_descriptor_handle_type=handle_alloc("usb_driver_descriptor",NULL);
	}
	handle_new(driver,usb_driver_descriptor_handle_type,&(driver->handle));
	handle_finish_setup(&(driver->handle));
}



void usb_unregister_driver(usb_driver_descriptor_t* driver){
	LOG("Unregistering USB driver '%s'...",driver->name);
	handle_destroy(&(driver->handle));
}
