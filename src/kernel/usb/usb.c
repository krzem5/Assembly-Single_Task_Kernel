#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/usb/device.h>
#include <kernel/usb/usb.h>
#define KERNEL_LOG_NAME "usb"



HANDLE_DECLARE_TYPE(USB_DRIVER_DESCRIPTOR,{});



void usb_register_driver(usb_driver_descriptor_t* driver){
	LOG("Registering USB driver '%s'...",driver->name);
	handle_new(driver,HANDLE_TYPE_USB_DRIVER_DESCRIPTOR,&(driver->handle));
}



void usb_unregister_driver(usb_driver_descriptor_t* driver){
	LOG("Unregistering USB driver '%s'...",driver->name);
	handle_destroy(&(driver->handle));
}



void usb_device_attach_driver(usb_device_t* device,usb_driver_t* driver){
	lock_acquire_exclusive(&(device->driver_list.lock));
	driver->prev=NULL;
	driver->next=device->driver_list.head;
	if (driver->next){
		driver->next->prev=driver;
	}
	device->driver_list.head=driver;
	lock_release_exclusive(&(device->driver_list.lock));
}



void usb_device_dettach_driver(usb_device_t* device,usb_driver_t* driver){
	lock_acquire_exclusive(&(device->driver_list.lock));
	if (driver->prev){
		driver->prev->next=driver->next;
	}
	else{
		device->driver_list.head=driver->next;
	}
	if (driver->next){
		driver->next->prev=driver->prev;
	}
	lock_release_exclusive(&(device->driver_list.lock));
}
