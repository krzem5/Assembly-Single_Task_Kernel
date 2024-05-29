#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/usb.h>
#define KERNEL_LOG_NAME "usb"



handle_type_t usb_driver_descriptor_handle_type=0;



KERNEL_INIT(){
	LOG("Initializing USB drivers...");
	usb_driver_descriptor_handle_type=handle_alloc("kernel.usb.driver.descriptor",NULL);
}



KERNEL_PUBLIC void usb_register_driver(usb_driver_descriptor_t* driver){
	LOG("Registering USB driver '%s'...",driver->name);
	handle_new(usb_driver_descriptor_handle_type,&(driver->handle));
	handle_finish_setup(&(driver->handle));
}



KERNEL_PUBLIC void usb_unregister_driver(usb_driver_descriptor_t* driver){
	LOG("Unregistering USB driver '%s'...",driver->name);
	handle_destroy(&(driver->handle));
}
