#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/usb/device.h>
#include <kernel/usb/pipe.h>
#include <kernel/usb/usb.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
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



typedef struct KERNEL_PACKED _USB_MSC_CBW{
	u32 dCBWSignature;
	u32 dCBWTag;
	u32 dCBWDataTransferLength;
	u8 bmCBWFlags;
	u8 bCBWLUN;
	u8 bCBWCBLength;
	union KERNEL_PACKED{
		u8 _data[16];
		struct KERNEL_PACKED{
			u8 type;
		} test_unit_ready;
		struct KERNEL_PACKED{
			u8 type;
			u8 _padding[3];
			u8 size;
		} inquiry;
		struct KERNEL_PACKED{
			u8 type;
		} read_capacity_10;
		struct KERNEL_PACKED{
			u8 type;
			u8 _padding;
			u32 lba;
			u8 _padding2;
			u16 count;
		} read_write_10;
	} CBWCB;
} usb_msc_cbw_t;



typedef struct KERNEL_PACKED _USB_MSC_CSW{
	u32 dCSWSignature;
	u32 dCSWTag;
	u32 dCSWDataResidue;
	u8 bCSWStatus;
} usb_msc_csw_t;



typedef struct KERNEL_PACKED _USB_SCSI_INQUIRY_RESPONCE{
	u8 pdt;
	u8 removable;
	u8 _padding[6];
	char vendor[8];
	char product[16];
	char rev[4];
} usb_scsi_inquiry_responce_t;



typedef struct KERNEL_PACKED _USB_SCSI_READ_CAPACITY_10_RESPONCE{
	u32 sectors;
	u32 block_size;
} usb_scsi_read_capacity_10_responce_t;



typedef struct _USB_MSC_LUN_CONTEXT{
	struct _USB_MSC_DRIVER* driver;
	struct _USB_MSC_LUN_CONTEXT* next;
	usb_msc_cbw_t cbw;
	usb_msc_csw_t csw;
	rwlock_t lock;
	u32 tag;
	u8 lun;
} usb_msc_lun_context_t;



typedef struct _USB_MSC_DRIVER{
	usb_driver_t driver;
	usb_device_t* device;
	usb_pipe_t* input_pipe;
	usb_pipe_t* output_pipe;
	rwlock_t lock;
	struct _USB_MSC_LUN_CONTEXT* lun_context;
} usb_msc_driver_t;



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _usb_msc_driver_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _usb_msc_driver_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _usb_msc_lun_context_allocator=NULL;

static usb_driver_descriptor_t _usb_msc_driver_descriptor;
static u16 _usb_msc_index=0;



static bool _fetch_inquiry(usb_msc_lun_context_t* context,usb_scsi_inquiry_responce_t* out){
	usb_msc_driver_t* driver=context->driver;
	rwlock_acquire_write(&(context->lock));
	mem_fill(context->cbw.CBWCB._data,16,0);
	context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
	context->cbw.dCBWTag=context->tag;
	context->cbw.dCBWDataTransferLength=sizeof(usb_scsi_inquiry_responce_t);
	context->cbw.bmCBWFlags=0x80;
	context->cbw.bCBWLUN=context->lun;
	context->cbw.bCBWCBLength=16;
	context->cbw.CBWCB.inquiry.type=USB_MSC_CBW_INQUIRY;
	context->cbw.CBWCB.inquiry.size=sizeof(usb_scsi_inquiry_responce_t);
	context->tag++;
	rwlock_acquire_write(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,out,sizeof(usb_scsi_inquiry_responce_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
	rwlock_release_write(&(driver->lock));
	bool ret=(context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus);
	rwlock_release_write(&(context->lock));
	return ret;
}



static bool _wait_for_device(usb_msc_lun_context_t* context){
	usb_msc_driver_t* driver=context->driver;
	rwlock_acquire_write(&(context->lock));
	while (1){
		mem_fill(context->cbw.CBWCB._data,16,0);
		context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
		context->cbw.dCBWTag=context->tag;
		context->cbw.dCBWDataTransferLength=0;
		context->cbw.bmCBWFlags=0x80;
		context->cbw.bCBWLUN=context->lun;
		context->cbw.bCBWCBLength=16;
		context->cbw.CBWCB.test_unit_ready.type=USB_MSC_CBW_TEST_UNIT_READY;
		context->tag++;
		rwlock_acquire_write(&(driver->lock));
		usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
		usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
		rwlock_release_write(&(driver->lock));
		if (context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus){
			rwlock_release_write(&(context->lock));
			return 1;
		}
		panic("_wait_for_device: send USB_MSC_CBW_REQUEST_SENSE");
	}
	rwlock_release_write(&(context->lock));
	return 0;
}



static bool _fetch_read_capacity_10(usb_msc_lun_context_t* context,usb_scsi_read_capacity_10_responce_t* out){
	usb_msc_driver_t* driver=context->driver;
	rwlock_acquire_write(&(context->lock));
	mem_fill(context->cbw.CBWCB._data,16,0);
	context->cbw.dCBWSignature=USB_MSC_CBW_SIGNATURE;
	context->cbw.dCBWTag=context->tag;
	context->cbw.dCBWDataTransferLength=sizeof(usb_scsi_read_capacity_10_responce_t);
	context->cbw.bmCBWFlags=0x80;
	context->cbw.bCBWLUN=context->lun;
	context->cbw.bCBWCBLength=16;
	context->cbw.CBWCB.read_capacity_10.type=USB_MSC_CBW_READ_CAPACITY_10;
	context->tag++;
	rwlock_acquire_write(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,out,sizeof(usb_scsi_read_capacity_10_responce_t));
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
	rwlock_release_write(&(driver->lock));
	bool ret=(context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus);
	rwlock_release_write(&(context->lock));
	return ret;
}



static u64 _usb_msc_read_write(drive_t* drive,u64 offset,u64 buffer,u64 count){
	usb_msc_lun_context_t* context=drive->extra_data;
	usb_msc_driver_t* driver=context->driver;
	rwlock_acquire_write(&(context->lock));
	mem_fill(context->cbw.CBWCB._data,16,0);
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
	rwlock_acquire_write(&(driver->lock));
	usb_pipe_transfer_normal(driver->device,driver->output_pipe,&(context->cbw),sizeof(usb_msc_cbw_t));
	usb_pipe_transfer_normal(driver->device,((offset&DRIVE_OFFSET_FLAG_WRITE)?driver->output_pipe:driver->input_pipe),(void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),count<<drive->block_size_shift);
	usb_pipe_transfer_normal(driver->device,driver->input_pipe,&(context->csw),sizeof(usb_msc_csw_t));
	rwlock_release_write(&(driver->lock));
	u64 out=(context->csw.dCSWSignature==USB_MSC_CSW_SIGNATURE&&context->csw.dCSWTag==context->cbw.dCBWTag&&!context->csw.bCSWStatus?((count<<drive->block_size_shift)-context->csw.dCSWDataResidue)>>drive->block_size_shift:0);
	rwlock_release_write(&(context->lock));
	return out;
}



static const drive_type_t _usb_msc_drive_type_config={
	"usb",
	0,
	_usb_msc_read_write
};



static void _setup_drive(usb_msc_driver_t* driver,u16 device_index,u8 lun){
	INFO("Setting up LUN %u...",lun);
	usb_msc_lun_context_t* context=omm_alloc(_usb_msc_lun_context_allocator);
	context->driver=driver;
	rwlock_init(&(context->lock));
	context->tag=0;
	context->lun=lun;
	void* buffer=(void*)(pmm_alloc(pmm_align_up_address(sizeof(usb_scsi_inquiry_responce_t)+sizeof(usb_scsi_read_capacity_10_responce_t))>>PAGE_SIZE_SHIFT,_usb_msc_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	usb_scsi_inquiry_responce_t* inquiry_data=buffer;
	usb_scsi_read_capacity_10_responce_t* read_capacity_10_data=buffer+sizeof(usb_scsi_inquiry_responce_t);
	if (!_fetch_inquiry(context,inquiry_data)||!_wait_for_device(context)||!_fetch_read_capacity_10(context,read_capacity_10_data)){
		goto _error;
	}
	context->next=driver->lun_context;
	driver->lun_context=context;
	char model_number_buffer[41];
	str_copy_from_padded(inquiry_data->product,model_number_buffer,16);
	drive_config_t config={
		&_usb_msc_drive_type_config,
		device_index,
		lun,
		smm_alloc("",0),
		smm_alloc(model_number_buffer,0),
		__builtin_bswap32(read_capacity_10_data->sectors),
		__builtin_bswap32(read_capacity_10_data->block_size),
		context
	};
	drive_create(&config);
	goto _cleanup;
_error:
	WARN("Failed to setup LUN %u",lun);
	omm_dealloc(_usb_msc_lun_context_allocator,context);
_cleanup:
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(sizeof(usb_scsi_inquiry_responce_t)+sizeof(usb_scsi_read_capacity_10_responce_t))>>PAGE_SIZE_SHIFT,_usb_msc_driver_pmm_counter);
}



static bool _usb_msc_load(usb_device_t* device,usb_interface_descriptor_t* interface_descriptor){
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
	usb_msc_driver_t* driver=omm_alloc(_usb_msc_driver_allocator);
	driver->driver.descriptor=&_usb_msc_driver_descriptor;
	driver->device=device;
	driver->input_pipe=usb_pipe_alloc(device,input_descriptor->address,input_descriptor->attributes,input_descriptor->max_packet_size);
	driver->output_pipe=usb_pipe_alloc(device,output_descriptor->address,output_descriptor->attributes,output_descriptor->max_packet_size);
	rwlock_init(&(driver->lock));
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
	u16 device_index=_usb_msc_index;
	_usb_msc_index++;
	for (u16 i=0;i<=max_lun;i++){
		_setup_drive(driver,device_index,i);
	}
	return 1;
}



static usb_driver_descriptor_t _usb_msc_driver_descriptor={
	"USB MSC",
	_usb_msc_load
};



MODULE_INIT(){
	_usb_msc_driver_pmm_counter=pmm_alloc_counter("usb_msc");
	_usb_msc_driver_allocator=omm_init("usb_msc_driver",sizeof(usb_msc_driver_t),8,1);
	rwlock_init(&(_usb_msc_driver_allocator->lock));
	_usb_msc_lun_context_allocator=omm_init("usb_msc_lun_context",sizeof(usb_msc_lun_context_t),8,1);
	rwlock_init(&(_usb_msc_lun_context_allocator->lock));
}



MODULE_POSTINIT(){
	LOG("Installing USB MSC driver...");
	usb_register_driver(&_usb_msc_driver_descriptor);
}
