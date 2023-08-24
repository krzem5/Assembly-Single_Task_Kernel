#include <kernel/drive/drive.h>
#include <kernel/driver/ata.h>
#include <kernel/format/format.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "ata"



#define TIMEOUT 0xffffff



// ATA registers
#define ATA_REG_DATA 0
#define ATA_REG_ERROR 1
#define ATA_REG_NSECTOR 2
#define ATA_REG_LBA0 3
#define ATA_REG_LBA1 4
#define ATA_REG_LBA2 5
#define ATA_REG_DRV_HEAD 6
#define ATA_REG_STATUS 7
#define ATA_REG_COMMAND 7
#define ATA_REG_DEV_CTL 518

// ATA commands
#define ATA_CMD_PACKET 0xa0
#define ATA_CMD_ATAPI_IDENTIFY 0xa1
#define ATA_CMD_IDENTIFY 0xec

// ATAPI commands
#define ATAPI_CMD_READ_CAPACITY 0x25
#define ATAPI_CMD_READ_SECTORS 0xa8

// Status flags
#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_BSY 0x80

// DEV_CTL flags
#define DEV_CTL_SRST 0x04



static KERNEL_CORE_RDATA const char _ata_drive_name_format_template[]="ata%u";



static inline void KERNEL_CORE_CODE KERNEL_NOCOVERAGE _delay_400ns(const ata_device_t* device){
	io_port_in8(device->port+ATA_REG_DEV_CTL);
	io_port_in8(device->port+ATA_REG_DEV_CTL);
	io_port_in8(device->port+ATA_REG_DEV_CTL);
	io_port_in8(device->port+ATA_REG_DEV_CTL);
}



static _Bool KERNEL_CORE_CODE KERNEL_NOCOVERAGE _wait_for_device(const ata_device_t* device,u8 mask,u8 value,u32 timeout){
	for (;timeout&&(io_port_in8(device->port+ATA_REG_STATUS)&mask)!=value;timeout--){
		if (io_port_in8(device->port+ATA_REG_STATUS)&STATUS_ERR){
			WARN_CORE("ATA/ATAPI device returned an error");
			return 0;
		}
		_delay_400ns(device);
	}
	return !!timeout;
}



static void KERNEL_CORE_CODE _send_atapi_command(const ata_device_t* device,const u16* command,u16 return_length,u16* output_buffer){
	io_port_out8(device->port+ATA_REG_DRV_HEAD,0xa0|(device->is_slave<<4));
	_delay_400ns(device);
	io_port_out8(device->port+ATA_REG_ERROR,0x00);
	io_port_out8(device->port+ATA_REG_LBA1,return_length);
	io_port_out8(device->port+ATA_REG_LBA2,return_length>>8);
	io_port_out8(device->port+ATA_REG_COMMAND,ATA_CMD_PACKET);
	_delay_400ns(device);
	if (!_wait_for_device(device,STATUS_DRQ|STATUS_BSY,STATUS_DRQ,TIMEOUT)){
		return;
	}
	for (u8 i=0;i<6;i++){
		io_port_out16(device->port+ATA_REG_DATA,command[i]);
	}
	_delay_400ns(device);
	if (output_buffer){
		if (!_wait_for_device(device,STATUS_DRQ|STATUS_BSY,STATUS_DRQ,TIMEOUT)){
			return;
		}
		for (u8 i=0;i<(return_length>>1);i++){
			output_buffer[i]=io_port_in16(device->port+ATA_REG_DATA);
		}
	}
}



static u64 KERNEL_CORE_CODE _ata_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	const ata_device_t* device=extra_data;
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		WARN_CORE("ATA drives are read-only");
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
	u16* out=buffer;
	for (u64 i=0;i<count;i++){
		_delay_400ns(device);
		if (!_wait_for_device(device,STATUS_BSY,0,TIMEOUT)){
			return 0;
		}
		u16 size=(io_port_in8(device->port+ATA_REG_LBA2)<<8)|io_port_in8(device->port+ATA_REG_LBA1);
		if (size!=2048){
			WARN_CORE("ATA drive returned sector not equal to 2048 bytes");
		}
		for (u16 j=0;j<2048;j+=2){
			*out=io_port_in16(device->port+ATA_REG_DATA);
			out++;
		}
	}
	if (!_wait_for_device(device,STATUS_DRQ|STATUS_BSY,0,TIMEOUT)){
		return 0;
	}
	return count;
}



static void KERNEL_CORE_CODE _ata_init(ata_device_t* device,u8 index){
	io_port_out8(device->port+ATA_REG_DEV_CTL,DEV_CTL_SRST);
	_delay_400ns(device);
	io_port_out8(device->port+ATA_REG_DEV_CTL,0);
	if (!_wait_for_device(device,STATUS_BSY,0,TIMEOUT)){
		goto _error;
	}
	io_port_out8(device->port+ATA_REG_DRV_HEAD,0xa0|(device->is_slave<<4));
	_delay_400ns(device);
	io_port_out8(device->port+ATA_REG_LBA0,0);
	io_port_out8(device->port+ATA_REG_LBA1,0);
	io_port_out8(device->port+ATA_REG_LBA2,0);
	io_port_out8(device->port+ATA_REG_COMMAND,ATA_CMD_IDENTIFY);
	if (!io_port_in8(device->port+7)){
		goto _error;
	}
	if (!_wait_for_device(device,STATUS_BSY,0,TIMEOUT)){
		goto _error;
	}
	u16 signature=(io_port_in8(device->port+ATA_REG_LBA1)<<8)|io_port_in8(device->port+ATA_REG_LBA2);
	switch (signature){
		case 0x0000:
			device->is_atapi=0;
			break;
		case 0x14eb:
			io_port_out8(device->port+ATA_REG_COMMAND,ATA_CMD_ATAPI_IDENTIFY);
			if (!_wait_for_device(device,STATUS_BSY,0,TIMEOUT)){
				goto _error;
			}
			device->is_atapi=1;
			break;
		case 0x3cc3:
		case 0x6996:
			goto _error;
		default:
			WARN_CORE("Invalid ATA signature '%x'; ignoring drive",signature);
			goto _error;
	}
	u16 buffer[256];
	for (u16 i=0;i<256;i++){
		buffer[i]=io_port_in16(device->port+ATA_REG_DATA);
	}
	if (!(buffer[49]&0x0200)){
		WARN_CORE("ATA/ATAPI drive does not support LBA; ignoring drive");
		goto _error;
	}
	drive_t drive={
		.type=(device->is_atapi?DRIVE_TYPE_ATAPI:DRIVE_TYPE_ATA),
		.read_write=_ata_read_write,
		.extra_data=device
	};
	format_string(drive.name,16,_ata_drive_name_format_template,index);
	bswap16_trunc_spaces(buffer+10,10,drive.serial_number);
	bswap16_trunc_spaces(buffer+27,20,drive.model_number);
	if (!device->is_atapi){
		WARN_CORE("Unimplemented: ATA drive");
		goto _error;
	}
	kmm_end_buffer();
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
	drive.block_count=((output_buffer[0]<<24)|(output_buffer[1]<<16)|(output_buffer[2]<<8)|output_buffer[3])+1;
	drive.block_size=(output_buffer[4]<<24)|(output_buffer[5]<<16)|(output_buffer[6]<<8)|output_buffer[7];
	drive_add(&drive);
	return;
_error:
	kmm_shrink_buffer(sizeof(ata_device_t));
	kmm_end_buffer();
}



void KERNEL_CORE_CODE driver_ata_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x01||device->progif!=0x80){
		return;
	}
	pci_device_enable_memory_access(device);
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,4,&pci_bar)){
		return;
	}
	LOG_CORE("Attached ATA driver to PCI device %x:%x:%x",device->bus,device->slot,device->func);
	for (u8 i=0;i<4;i++){
		ata_device_t* ata_device=kmm_alloc_buffer();
		kmm_grow_buffer(sizeof(ata_device_t));
		ata_device->dma_address=pci_bar.address;
		ata_device->is_slave=i&1;
		ata_device->port=((i>>1)?0x170:0x1f0);
		_ata_init(ata_device,i);
	}
}
