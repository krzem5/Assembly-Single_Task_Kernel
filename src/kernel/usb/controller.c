#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/usb/address_space.h>
#include <kernel/usb/controller.h>
#define KERNEL_LOG_NAME "usb_controller"



static omm_allocator_t* _usb_root_controller_allocator=NULL;
static omm_allocator_t* _usb_controller_allocator=NULL;



KERNEL_INIT(){
	LOG("Initializing USB controllers...");
	_usb_root_controller_allocator=omm_init("usb_root_controller",sizeof(usb_root_controller_t),8,2,pmm_alloc_counter("omm_usb_root_controller"));
	spinlock_init(&(_usb_root_controller_allocator->lock));
	_usb_controller_allocator=omm_init("usb_controller",sizeof(usb_controller_t),8,2,pmm_alloc_counter("omm_usb_controller"));
	spinlock_init(&(_usb_controller_allocator->lock));
}



KERNEL_PUBLIC usb_root_controller_t* usb_root_controller_alloc(void){
	usb_root_controller_t* out=omm_alloc(_usb_root_controller_allocator);
	usb_address_space_init(&(out->address_space));
	return out;
}



KERNEL_PUBLIC usb_controller_t* usb_controller_alloc(usb_root_controller_t* root_controller){
	usb_controller_t* out=omm_alloc(_usb_controller_allocator);
	out->root_controller=root_controller;
	return out;
}



KERNEL_PUBLIC void usb_root_controller_dealloc(usb_root_controller_t* root_controller){
	omm_dealloc(_usb_root_controller_allocator,root_controller);
}



KERNEL_PUBLIC void usb_controller_dealloc(usb_controller_t* controller){
	omm_dealloc(_usb_controller_allocator,controller);
}
