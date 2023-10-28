#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/usb.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "usb_msc_driver"



typedef struct _USB_MSC_DRIVER{
	usb_driver_t driver;
	usb_pipe_t* input_pipe;
	usb_pipe_t* output_pipe;
} usb_msc_driver_t;



static pmm_counter_descriptor_t _usb_msc_driver_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_msc_driver");
static omm_allocator_t _usb_msc_driver_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_msc_driver",sizeof(usb_msc_driver_t),8,1,&_usb_msc_driver_omm_pmm_counter);



static usb_driver_descriptor_t _usb_msc_driver_descriptor;



static _Bool _usb_msc_load(usb_device_t* device,usb_interface_descriptor_t* interface_descriptor){
	if (interface_descriptor->class!=0x08||interface_descriptor->subclass!=0x06||interface_descriptor->protocol!=0x50){
		return 0;
	}
	usb_endpoint_descriptor_t* input_descriptor=NULL;
	usb_endpoint_descriptor_t* output_descriptor=NULL;
	for (usb_endpoint_descriptor_t* endpoint_descriptor=interface_descriptor->endpoint;endpoint_descriptor;endpoint_descriptor=endpoint_descriptor->next){
		if ((endpoint_descriptor->attributes&USB_ENDPOINT_XFER_MASK)!=USB_ENDPOINT_XFER_BULK){
			continue;
		}
		if (!input_descriptor&&(endpoint_descriptor->address&USB_DIR_IN)){
			input_descriptor=endpoint_descriptor;
		}
		if (!output_descriptor&&!(endpoint_descriptor->address&USB_DIR_IN)){
			output_descriptor=endpoint_descriptor;
		}
	}
	if (!input_descriptor||!output_descriptor){
		return 0;
	}
	usb_msc_driver_t* driver=omm_alloc(&_usb_msc_driver_allocator);
	driver->driver.descriptor=&_usb_msc_driver_descriptor;
	driver->input_pipe=usb_pipe_alloc(device,input_descriptor->address,input_descriptor->attributes,input_descriptor->max_packet_size);
	driver->output_pipe=usb_pipe_alloc(device,output_descriptor->address,output_descriptor->attributes,output_descriptor->max_packet_size);
	interface_descriptor->driver=(usb_driver_t*)driver;
	usb_raw_control_request_t request={
		USB_DIR_IN|USB_TYPE_CLASS|USB_RECIP_INTERFACE,
		0xfe,
		0,
		0,
		1
	};
	u8 max_lun;
	usb_pipe_transfer_setup(device,device->default_pipe,&request,&max_lun);
	WARN("Max LUN: %u",max_lun);
	return 1;
}



static usb_driver_descriptor_t _usb_msc_driver_descriptor={
	"USB MSC",
	_usb_msc_load
};



void usb_msc_driver_install(void){
	LOG("Installing USB MSC driver...");
	usb_register_driver(&_usb_msc_driver_descriptor);
}
