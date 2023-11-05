#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/usb.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "usb_msc_driver"



#define USB_MSC_CBW_TEST_UNIT_READY 0x00
#define USB_MSC_CBW_INQUIRY 0x12
#define USB_MSC_CBW_MODE_SELECT_6 0x15
#define USB_MSC_CBW_MODE_SENSE_6 0x1a
#define USB_MSC_CBW_START_STOP_UNIT 0x1b
#define USB_MSC_CBW_PREVENT_ALLOW_MEDIUM_REMOVAL 0x1e
#define USB_MSC_CBW_READ_CAPACITY_10 0x25
#define USB_MSC_CBW_REQUEST_SENSE 0x03
#define USB_MSC_CBW_READ_FORMAT_CAPACITY 0x23
#define USB_MSC_CBW_READ_10 0x28
#define USB_MSC_CBW_WRITE_10 0x2a

#define USB_MSC_CBW_SIGNATURE 0x43425355
#define USB_MSC_CSW_SIGNATURE 0x53425355



typedef struct __attribute__((packed)) _USB_MSC_CBW{
	u32 dCBWSignature;
	u32 dCBWTag;
	u32 dCBWDataTransferLength;
	u8 bmCBWFlags;
	u8 bCBWLUN;
	u8 bCBWCBLength;
	union __attribute__((packed)){
		u8 _data[16];
		struct __attribute__((packed)){
			u8 type;
		} test_unit_ready;
		struct __attribute__((packed)){
			u8 type;
			u8 _padding[3];
			u8 size;
		} inquiry;
		struct __attribute__((packed)){
			u8 type;
		} read_capacity_10;
		struct __attribute__((packed)){
			u8 type;
			u8 _padding;
			u32 lba;
			u8 _padding2;
			u16 count;
		} read_write_10;
	} CBWCB;
} usb_msc_cbw_t;



typedef struct __attribute__((packed)) _USB_MSC_CSW{
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8 bCSWStatus;
} usb_msc_csw_t;



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
	usb_msc_cbw_t cbw;
	usb_msc_csw_t csw;
	spinlock_t lock;
	u32 tag;
	u8 lun;
} usb_msc_lun_context_t;



typedef struct _USB_MSC_DRIVER{
	usb_driver_t driver;
	usb_device_t* device;
	usb_pipe_t* input_pipe;
	usb_pipe_t* output_pipe;
	spinlock_t lock;
	struct _USB_MSC_LUN_CONTEXT* lun_context;
} usb_msc_driver_t;



static pmm_counter_descriptor_t _usb_msc_driver_pmm_counter=PMM_COUNTER_INIT_STRUCT("usb_msc");
static pmm_counter_descriptor_t _usb_msc_driver_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_msc_driver");
static pmm_counter_descriptor_t _usb_msc_lun_context_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_usb_msc_lun_context");
static omm_allocator_t _usb_msc_driver_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_msc_driver",sizeof(usb_msc_driver_t),8,1,&_usb_msc_driver_omm_pmm_counter);
static omm_allocator_t _usb_msc_lun_context_allocator=OMM_ALLOCATOR_INIT_STRUCT("usb_msc_lun_context",sizeof(usb_msc_lun_context_t),8,1,&_usb_msc_lun_context_omm_pmm_counter);



static usb_driver_descriptor_t _usb_msc_driver_descriptor;



static u16 _usb_msc_index=0;



static _Bool _fetch_inquiry(usb_msc_lun_context_t* context,usb_scsi_inquiry_responce_t* out){
	usb_msc_driver_t* driver=context->driver;
	spinlock_acquire_exclusive(&(context->lock));
	memset(context->cbw.CBWCB._data,0,16);
	context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
	context->cbw.dCBWTag=context->tag;
	context->cbw.dCBWDataTransferLength=sizeof(usb_scsi_inquiry_responce_t);
	context->cbw.bmCBWFlags=0x80;
	context->cbw.bCBWLUN=context->lun;
	context->cbw.bCBWCBLength=16;
	context->cbw.CBWCB.inquiry.type=USB_MSC_CBW_INQUIRY;
	context->cbw.CBWCB.inquiry.size=sizeof(usb_scsi_inquiry_responce_t);
	context->tag++;
	spinlock_acquire_exclusive(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,out,sizeof(usb_scsi_inquiry_responce_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
	spinlock_release_exclusive(&(driver->lock));
	_Bool ret=(context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus);
	spinlock_release_exclusive(&(context->lock));
	return ret;
}



static _Bool _wait_for_device(usb_msc_lun_context_t* context){
	usb_msc_driver_t* driver=context->driver;
	spinlock_acquire_exclusive(&(context->lock));
	while (1){
		memset(context->cbw.CBWCB._data,0,16);
		context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
		context->cbw.dCBWTag=context->tag;
		context->cbw.dCBWDataTransferLength=0;
		context->cbw.bmCBWFlags=0x80;
		context->cbw.bCBWLUN=context->lun;
		context->cbw.bCBWCBLength=16;
		context->cbw.CBWCB.test_unit_ready.type=USB_MSC_CBW_TEST_UNIT_READY;
		context->tag++;
		spinlock_acquire_exclusive(&(driver->lock));
		usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
		usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
		spinlock_release_exclusive(&(driver->lock));
		if (context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus){
			spinlock_release_exclusive(&(context->lock));
			return 1;
		}
		panic("_wait_for_device: send USB_MSC_CBW_REQUEST_SENSE");
	}
	spinlock_release_exclusive(&(context->lock));
	return 0;
}



static _Bool _fetch_read_capacity_10(usb_msc_lun_context_t* context,usb_scsi_read_capacity_10_responce_t* out){
	usb_msc_driver_t* driver=context->driver;
	spinlock_acquire_exclusive(&(context->lock));
	memset(context->cbw.CBWCB._data,0,16);
	context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
	context->cbw.dCBWTag=context->tag;
	context->cbw.dCBWDataTransferLength=sizeof(usb_scsi_read_capacity_10_responce_t);
	context->cbw.bmCBWFlags=0x80;
	context->cbw.bCBWLUN=context->lun;
	context->cbw.bCBWCBLength=16;
	context->cbw.CBWCB.read_capacity_10.type=USB_MSC_CBW_READ_CAPACITY_10;
	context->tag++;
	spinlock_acquire_exclusive(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,out,sizeof(usb_scsi_read_capacity_10_responce_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
	spinlock_release_exclusive(&(driver->lock));
	_Bool ret=(context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus);
	spinlock_release_exclusive(&(context->lock));
	return ret;
}



static u64 _usb_msc_read_write(drive_t* drive,u64 offset,void* buffer,u64 count){
	usb_msc_lun_context_t* context=drive->extra_data;
	usb_msc_driver_t* driver=context->driver;
	void* linear_buffer=(void*)(pmm_alloc(pmm_align_up_address(count<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,&_usb_msc_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		memcpy(linear_buffer,buffer,count<<drive->block_size_shift);
	}
	spinlock_acquire_exclusive(&(context->lock));
	memset(context->cbw.CBWCB._data,0,16);
	context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
	context->cbw.dCBWTag=context->tag;
	context->cbw.dCBWDataTransferLength=count<<drive->block_size_shift;
	context->cbw.bmCBWFlags=((offset&DRIVE_OFFSET_FLAG_WRITE)?0x00:0x80);
	context->cbw.bCBWLUN=context->lun;
	context->cbw.bCBWCBLength=16;
	context->cbw.CBWCB.read_write_10.type=((offset&DRIVE_OFFSET_FLAG_WRITE)?USB_MSC_CBW_WRITE_10:USB_MSC_CBW_READ_10);
	context->cbw.CBWCB.read_write_10.lba=__builtin_bswap32(offset);
	context->cbw.CBWCB.read_write_10.count=__builtin_bswap16((u16)count);
	context->tag++;
	spinlock_acquire_exclusive(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
	usb_pipe_transfer_normal(driver->device,((offset&DRIVE_OFFSET_FLAG_WRITE)?driver->output_pipe:driver->input_pipe),linear_buffer,count<<drive->block_size_shift);
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
	spinlock_release_exclusive(&(driver->lock));
	if (!(offset&DRIVE_OFFSET_FLAG_WRITE)){
		memcpy(buffer,linear_buffer,count<<drive->block_size_shift);
	}
	u64 out=(context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus?((count<<drive->block_size_shift)-context->csw.dCSWDataResidue)>>drive->block_size_shift:0);
	spinlock_release_exclusive(&(context->lock));
	pmm_dealloc(((u64)linear_buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(count<<drive->block_size_shift)>>PAGE_SIZE_SHIFT,&_usb_msc_driver_pmm_counter);
	return out;
}



static drive_type_t _usb_msc_drive_type={
	"USB MSC",
	_usb_msc_read_write
};



static void _setup_drive(usb_msc_driver_t* driver,u8 lun){
	INFO("Setting up LUN %u...",lun);
	usb_msc_lun_context_t* context=omm_alloc(&_usb_msc_lun_context_allocator);
	context->driver=driver;
	spinlock_init(&(context->lock));
	context->tag=0;
	context->lun=lun;
	void* buffer=(void*)(pmm_alloc(pmm_align_up_address(sizeof(usb_scsi_inquiry_responce_t)+sizeof(usb_scsi_read_capacity_10_responce_t))>>PAGE_SIZE_SHIFT,&_usb_msc_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	usb_scsi_inquiry_responce_t* inquiry_data=buffer;
	usb_scsi_read_capacity_10_responce_t* read_capacity_10_data=buffer+sizeof(usb_scsi_inquiry_responce_t);
	if (!_fetch_inquiry(context,inquiry_data)||!_wait_for_device(context)||!_fetch_read_capacity_10(context,read_capacity_10_data)){
		goto _error;
	}
	context->next=driver->lun_context;
	driver->lun_context=context;
	drive_config_t config={
		.type=&_usb_msc_drive_type,
		.block_count=__builtin_bswap32(read_capacity_10_data->sectors),
		.block_size=__builtin_bswap32(read_capacity_10_data->block_size),
		.extra_data=context
	};
	format_string(config.name,DRIVE_NAME_LENGTH,"usb%ud%u",_usb_msc_index,lun);
	_usb_msc_index++;
	memcpy_trunc_spaces(config.serial_number,inquiry_data->rev,4);
	memcpy_trunc_spaces(config.model_number,inquiry_data->product,16);
	drive_create(&config);
	goto _cleanup;
_error:
	WARN("Failed to setup LUN %u",lun);
	omm_dealloc(&_usb_msc_lun_context_allocator,context);
_cleanup:
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(sizeof(usb_scsi_inquiry_responce_t)+sizeof(usb_scsi_read_capacity_10_responce_t))>>PAGE_SIZE_SHIFT,&_usb_msc_driver_pmm_counter);
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
	spinlock_init(&(driver->lock));
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
