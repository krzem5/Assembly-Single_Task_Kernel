#include <ata/device.h>
#include <ata/registers.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "ata"



#define OPERATION_TIMEOUT 0xffffff



static omm_allocator_t* KERNEL_INIT_WRITE _ata_device_allocator=NULL;

static u16 _ata_device_index=0;



static KERNEL_INLINE void KERNEL_NOCOVERAGE _delay_400ns(const ata_device_t* device){
	io_port_in8(device->port+ATA_REG_DEV_CTL);
	io_port_in8(device->port+ATA_REG_DEV_CTL);
	io_port_in8(device->port+ATA_REG_DEV_CTL);
	io_port_in8(device->port+ATA_REG_DEV_CTL);
}



static _Bool KERNEL_NOCOVERAGE _wait_for_device(const ata_device_t* device,u8 mask,u8 value){
	u32 timeout=OPERATION_TIMEOUT;
	for (;timeout&&(io_port_in8(device->port+ATA_REG_STATUS)&mask)!=value;timeout--){
		if (io_port_in8(device->port+ATA_REG_STATUS)&STATUS_ERR){
			WARN("ATA/ATAPI device returned an error");
			return 0;
		}
		_delay_400ns(device);
	}
	return !!timeout;
}



static void _send_atapi_command(const ata_device_t* device,const u16* command,u16 return_length,u16* output_buffer){
	io_port_out8(device->port+ATA_REG_DRV_HEAD,0xa0|(device->is_slave<<4));
	_delay_400ns(device);
	io_port_out8(device->port+ATA_REG_ERROR,0x00);
	io_port_out8(device->port+ATA_REG_LBA1,return_length);
	io_port_out8(device->port+ATA_REG_LBA2,return_length>>8);
	io_port_out8(device->port+ATA_REG_COMMAND,ATA_CMD_PACKET);
	_delay_400ns(device);
	if (!_wait_for_device(device,STATUS_DRQ|STATUS_BSY,STATUS_DRQ)){
		return;
	}
	for (u8 i=0;i<6;i++){
		io_port_out16(device->port+ATA_REG_DATA,command[i]);
	}
	_delay_400ns(device);
	if (output_buffer){
		if (!_wait_for_device(device,STATUS_DRQ|STATUS_BSY,STATUS_DRQ)){
			return;
		}
		for (u8 i=0;i<(return_length>>1);i++){
			output_buffer[i]=io_port_in16(device->port+ATA_REG_DATA);
		}
	}
}



static u64 _ata_read_write(drive_t* drive,u64 offset,u64 buffer,u64 count){
	panic("_ata_read_write");
}



static u64 _atapi_read_write(drive_t* drive,u64 offset,u64 buffer,u64 count){
	const ata_device_t* device=drive->extra_data;
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		WARN("ATA drives are read-only");
		return 0;
	}
	offset&=DRIVE_OFFSET_MASK;
	if (count>0xffffffff){
		count=0xffffffff;
	}
	u8 atapi_command[12]={
		ATAPI_CMD_READ_SECTORS,
		0x00,
		offset>>24,
		offset>>16,
		offset>>8,
		offset,
		count>>24,
		count>>16,
		count>>8,
		count,
		0x00,
		0x00
	};
	_send_atapi_command(device,(const u16*)atapi_command,2048,NULL);
	u16* out=(void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u64 i=0;i<count;i++){
		_delay_400ns(device);
		if (!_wait_for_device(device,STATUS_BSY,0)){
			return 0;
		}
		u16 size=(io_port_in8(device->port+ATA_REG_LBA2)<<8)|io_port_in8(device->port+ATA_REG_LBA1);
		if (size!=2048){
			WARN("ATA drive returned sector not equal to 2048 bytes");
		}
		for (u16 j=0;j<2048;j+=2){
			*out=io_port_in16(device->port+ATA_REG_DATA);
			out++;
		}
	}
	if (!_wait_for_device(device,STATUS_DRQ|STATUS_BSY,0)){
		return 0;
	}
	return count;
}



static const drive_type_t _ata_drive_type_config={
	"ata",
	DRIVE_TYPE_FLAG_READ_ONLY,
	_ata_read_write
};

static const drive_type_t _atapi_drive_type_config={
	"atapi",
	DRIVE_TYPE_FLAG_READ_ONLY,
	_atapi_read_write
};



static _Bool _ata_init(ata_device_t* device,u8 index){
	io_port_out8(device->port+ATA_REG_DEV_CTL,DEV_CTL_SRST);
	_delay_400ns(device);
	io_port_out8(device->port+ATA_REG_DEV_CTL,0);
	if (!_wait_for_device(device,STATUS_BSY,0)){
		return 0;
	}
	io_port_out8(device->port+ATA_REG_DRV_HEAD,0xa0|(device->is_slave<<4));
	_delay_400ns(device);
	io_port_out8(device->port+ATA_REG_LBA0,0);
	io_port_out8(device->port+ATA_REG_LBA1,0);
	io_port_out8(device->port+ATA_REG_LBA2,0);
	io_port_out8(device->port+ATA_REG_COMMAND,ATA_CMD_IDENTIFY);
	if (!io_port_in8(device->port+7)){
		return 0;
	}
	if (!_wait_for_device(device,STATUS_BSY,0)){
		return 0;
	}
	u16 signature=(io_port_in8(device->port+ATA_REG_LBA1)<<8)|io_port_in8(device->port+ATA_REG_LBA2);
	switch (signature){
		case 0x0000:
			device->is_atapi=0;
			break;
		case 0x14eb:
			io_port_out8(device->port+ATA_REG_COMMAND,ATA_CMD_ATAPI_IDENTIFY);
			if (!_wait_for_device(device,STATUS_BSY,0)){
				return 0;
			}
			device->is_atapi=1;
			break;
		case 0x3cc3:
		case 0x6996:
			return 0;
		default:
			WARN("Invalid ATA signature '%x'; ignoring drive",signature);
			return 0;
	}
	u16 buffer[256];
	for (u16 i=0;i<256;i++){
		buffer[i]=io_port_in16(device->port+ATA_REG_DATA);
	}
	if (!(buffer[49]&0x0200)){
		WARN("ATA/ATAPI drive does not support LBA; ignoring drive");
		return 0;
	}
	if (!device->is_atapi){
		WARN("Unimplemented: ATA drive");
		return 0;
	}
	const u8 atapi_command[12]={
		ATAPI_CMD_READ_CAPACITY,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00
	};
	u8 output_buffer[8];
	_send_atapi_command(device,(const u16*)atapi_command,8,(u16*)output_buffer);
	char serial_number_buffer[21];
	char model_number_buffer[41];
	str_copy_byte_swap_from_padded(buffer+10,serial_number_buffer,10);
	str_copy_byte_swap_from_padded(buffer+27,model_number_buffer,20);
	drive_config_t config={
		(device->is_atapi?&_atapi_drive_type_config:&_ata_drive_type_config),
		_ata_device_index,
		index,
		smm_alloc(serial_number_buffer,0),
		smm_alloc(model_number_buffer,0),
		((output_buffer[0]<<24)|(output_buffer[1]<<16)|(output_buffer[2]<<8)|output_buffer[3])+1,
		(output_buffer[4]<<24)|(output_buffer[5]<<16)|(output_buffer[6]<<8)|output_buffer[7],
		device
	};
	drive_create(&config);
	return 1;
}



static void _ata_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x01||device->progif!=0x80){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,4,&pci_bar)){
		return;
	}
	LOG("Attached ATA driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	for (u8 i=0;i<4;i++){
		ata_device_t* ata_device=omm_alloc(_ata_device_allocator);
		ata_device->dma_address=(void*)(pci_bar.address);
		ata_device->is_slave=i&1;
		ata_device->port=((i>>1)?0x170:0x1f0);
		if (!_ata_init(ata_device,i)){
			omm_dealloc(_ata_device_allocator,ata_device);
		}
	}
	_ata_device_index++;
}



MODULE_INIT(){
	_ata_device_allocator=omm_init("ata_device",sizeof(ata_device_t),8,1);
	spinlock_init(&(_ata_device_allocator->lock));
}



MODULE_POSTINIT(){
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_ata_init_device(device);
	}
}



MODULE_DEINIT(){
	omm_deinit(_ata_device_allocator);
}
