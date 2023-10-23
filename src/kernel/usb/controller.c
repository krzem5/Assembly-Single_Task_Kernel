#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/usb/controller.h>



static pmm_counter_descriptor_t _usb_controller_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_controller");
static omm_allocator_t _usb_controller_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_controller",sizeof(usb_controller_t),8,2,&_usb_controller_omm_pmm_counter);



usb_controller_t* usb_controller_alloc(void){
	return omm_alloc(&_usb_controller_allocator);
}



void usb_controller_dealloc(usb_controller_t* controller){
	omm_dealloc(&_usb_controller_allocator,controller);
}
