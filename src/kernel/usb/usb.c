#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/usb/device.h>
#include <kernel/usb/usb.h>
#define KERNEL_LOG_NAME "usb"



HANDLE_DECLARE_TYPE(USB_DRIVER_DESCRIPTOR,{});



void usb_register_driver(usb_driver_descriptor_t* driver){
	LOG("Registering USB driver '%s'...",driver->name);
	handle_new(driver,HANDLE_TYPE_USB_DRIVER_DESCRIPTOR,&(driver->handle));
	handle_finish_setup(&(driver->handle));
}



void usb_unregister_driver(usb_driver_descriptor_t* driver){
	LOG("Unregistering USB driver '%s'...",driver->name);
	handle_destroy(&(driver->handle));
}
