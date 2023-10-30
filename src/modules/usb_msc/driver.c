#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/usb.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "usb_msc_driver"



#define USB_SCSI_COMMAND_TEST_UNIT_READY 0x00
#define USB_SCSI_COMMAND_INQUIRY 0x12
#define USB_SCSI_COMMAND_MODE_SELECT_6 0x15
#define USB_SCSI_COMMAND_MODE_SENSE_6 0x1a
#define USB_SCSI_COMMAND_START_STOP_UNIT 0x1b
#define USB_SCSI_COMMAND_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define USB_SCSI_COMMAND_READ_CAPACITY_10 0x25
#define USB_SCSI_COMMAND_REQUEST_SENSE 0x03
#define USB_SCSI_COMMAND_READ_FORMAT_CAPACITY 0x23
#define USB_SCSI_COMMAND_READ_10 0x28
#define USB_SCSI_COMMAND_WRITE_10 0x2a

#define USB_SCSI_COMMAND_SIGNATURE 0x43425355
#define USB_SCSI_STATUS_SIGNATURE 0x53425355



typedef struct __attribute__((packed)) _USB_SCSI_COMMAND{
	u32 signature;
	u32 tag;
	u32 size;
	u8 direction;
	u8 lun;
	u8 length;
	union __attribute__((packed)){
		u8 _data[16];
		struct __attribute__((packed)){
			u8 type;
			u8 _padding[3];
			u8 size;
		} inquiry;
		struct __attribute__((packed)){
			u8 type;
		} read_capacity_10;
	} command;
} usb_scsi_command_t;



typedef struct __attribute__((packed)) _USB_SCSI_STATUS{
	u32 signature;
	u32 tag;
	u32 residue;
	u8 status;
} usb_scsi_status_t;



typedef struct __attribute__((packed)) _USB_SCSI_INQUIRY_RESPONCE{
	u8 pdt;
	u8 removable;
	u8 _padding[6];
	char vendor[8];
	char product[16];
	char rev[4];
} usb_scsi_inquiry_responce_t;



typedef struct __attribute__((packed)) _USB_SCSI_READ_CAPACITY_10_RESPONCE{
	u32 sectors;
	u32 block_size;
} usb_scsi_read_capacity_10_responce_t;



typedef struct _USB_MSC_LUN_CONTEXT{
	struct _USB_MSC_DRIVER* driver;
	struct _USB_MSC_LUN_CONTEXT* next;
	u8 lun;
} usb_msc_lun_context_t;



typedef struct _USB_MSC_DRIVER{
	usb_driver_t driver;
	usb_device_t* device;
	usb_pipe_t* input_pipe;
	usb_pipe_t* output_pipe;
	lock_t lock;
	struct _USB_MSC_LUN_CONTEXT* lun_context;
} usb_msc_driver_t;



static pmm_counter_descriptor_t _usb_msc_driver_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_msc_driver");
static pmm_counter_descriptor_t _usb_msc_lun_context_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_msc_lun_context");
static omm_allocator_t _usb_msc_driver_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_msc_driver",sizeof(usb_msc_driver_t),8,1,&_usb_msc_driver_omm_pmm_counter);
static omm_allocator_t _usb_msc_lun_context_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_msc_lun_context",sizeof(usb_msc_lun_context_t),8,1,&_usb_msc_lun_context_omm_pmm_counter);



static usb_driver_descriptor_t _usb_msc_driver_descriptor;



static _Bool _fetch_inquiry(usb_msc_driver_t* driver,u8 lun,usb_scsi_inquiry_responce_t* out){
	usb_scsi_command_t command={
		USB_SCSI_COMMAND_SIGNATURE,
		0x11223344,
		sizeof(usb_scsi_inquiry_responce_t),
		USB_DIR_IN,
		lun,
		16,
		{
			.inquiry={
				.type=USB_SCSI_COMMAND_INQUIRY,
				.size=sizeof(usb_scsi_inquiry_responce_t)
			}
		}
	};
	lock_acquire_exclusive(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&command,sizeof(usb_scsi_command_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,out,sizeof(usb_scsi_inquiry_responce_t));
	usb_scsi_status_t status;
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&status,sizeof(usb_scsi_status_t));
	lock_release_exclusive(&(driver->lock));
	return (status.signature==USB_SCSI_STATUS_SIGNATURE&&status.tag==command.tag&&!status.status);
}



static _Bool _fetch_read_capacity_10(usb_msc_driver_t* driver,u8 lun,usb_scsi_read_capacity_10_responce_t* out){
	usb_scsi_command_t command={
		USB_SCSI_COMMAND_SIGNATURE,
		0x11223344,
		sizeof(usb_scsi_read_capacity_10_responce_t),
		USB_DIR_IN,
		lun,
		16,
		{
			.read_capacity_10={
				.type=USB_SCSI_COMMAND_READ_CAPACITY_10
			}
		}
	};
	lock_acquire_exclusive(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&command,sizeof(usb_scsi_command_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,out,sizeof(usb_scsi_read_capacity_10_responce_t));
	usb_scsi_status_t status;
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&status,sizeof(usb_scsi_status_t));
	lock_release_exclusive(&(driver->lock));
	return (status.signature==USB_SCSI_STATUS_SIGNATURE&&status.tag==command.tag&&!status.status);
}



static u64 _usb_msc_read_write(drive_t* drive,u64 offset,void* buffer,u64 count){
	return 0;
}



static drive_type_t _usb_msc_drive_type={
	"USB MSC",
	_usb_msc_read_write
};



static void _setup_drive(usb_msc_driver_t* driver,u8 lun){
	INFO("Setting up LUN %u...",lun);
	usb_scsi_inquiry_responce_t inquiry_data;
	usb_scsi_read_capacity_10_responce_t read_capacity_10_data;
	if (!_fetch_inquiry(driver,lun,&inquiry_data)||!_fetch_read_capacity_10(driver,lun,&read_capacity_10_data)){
		WARN("Failed to setup LUN %u",lun);
		return;
	}
	usb_msc_lun_context_t* context=omm_alloc(&_usb_msc_lun_context_allocator);
	context->driver=driver;
	context->next=driver->lun_context;
	context->lun=lun;
	driver->lun_context=context;
	drive_config_t config={
		.type=&_usb_msc_drive_type,
		.block_count=__builtin_bswap32(read_capacity_10_data.sectors),
		.block_size=__builtin_bswap32(read_capacity_10_data.block_size),
		.extra_data=context
	};
	format_string(config.name,DRIVE_NAME_LENGTH,"usb%u",lun);
	memcpy_trunc_spaces(config.serial_number,inquiry_data.rev,4);
	memcpy_trunc_spaces(config.model_number,inquiry_data.product,16);
	drive_create(&config);
}



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
	driver->device=device;
	driver->input_pipe=usb_pipe_alloc(device,input_descriptor->address,input_descriptor->attributes,input_descriptor->max_packet_size);
	driver->output_pipe=usb_pipe_alloc(device,output_descriptor->address,output_descriptor->attributes,output_descriptor->max_packet_size);
	lock_init(&(driver->lock));
	driver->lun_context=NULL;
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
	for (u16 i=0;i<=max_lun;i++){
		_setup_drive(driver,i);
	}
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
