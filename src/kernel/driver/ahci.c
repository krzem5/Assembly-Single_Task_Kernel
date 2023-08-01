#include <kernel/drive/drive.h>
#include <kernel/drive/drive_list.h>
#include <kernel/driver/ahci.h>
#include <kernel/log/log.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "ahci"



#define MAX_CONTROLLER_COUNT 1
#define MAX_DEVICE_COUNT 32



// Controller
#define CAP_NP 0x0000001f
#define CAP_NCS 0x00001f00
#define CAP_NCS_SHIFT 8
#define CAP_S64A 0x80000000

#define GHC_HR 0x00000001
#define GHC_IE 0x00000002
#define GHC_AE 0x80000000

#define CAP2_BOH 0x1

#define BOHC_BOS 0x01
#define BOHC_OOS 0x02
#define BOHC_BB 0x10

// Device
#define CMD_ST 0x0001
#define CMD_FRE 0x0010
#define CMD_CR 0x8000

#define TFD_STS_DSQ 0x08
#define TFD_STS_BSY 0x80

// Command
#define FLAGS_WRITE 0x40
#define FLAGS_PREFEACHABLE 0x80

// FIS
#define FIS_TYPE_REG_H2D 0x27

// FIS flags
#define FIS_FLAG_COMMAND 0x80

// ATA
#define ATA_CMD_IDENTIFY 0xec
#define ATA_CMD_READ 0x25
#define ATA_CMD_WRITE 0x35



static ahci_controller_t KERNEL_CORE_DATA _ahci_controllers[MAX_CONTROLLER_COUNT];
static u32 KERNEL_CORE_DATA _ahci_controller_count;
static ahci_device_t* KERNEL_CORE_DATA _ahci_devices;
static u32 KERNEL_CORE_DATA _ahci_device_count;



static u8 KERNEL_CORE_CODE _device_get_command_slot(const ahci_device_t* device){
	u8 i=0;
	while (1){
		if (!((device->registers->sact|device->registers->ci)&(1<<i))){
			return i;
		}
		i++;
		if (i==device->controller->command_slot_count){
			i=0;
		}
	}
	// u32 mask;
	// do{
	// 	mask=~(device->registers->sact|device->registers->ci);
	// } while (!mask);
	// return __builtin_ctz(mask);
}



static void KERNEL_CORE_CODE _device_send_command(const ahci_device_t* device,u8 cmd_slot){
	while (device->registers->tfd&(TFD_STS_DSQ|TFD_STS_BSY)){
		asm volatile("pause");
	}
	device->registers->cmd&=~CMD_ST;
	while (device->registers->cmd&CMD_CR){
		asm volatile("pause");
	}
	device->registers->cmd|=CMD_ST|CMD_FRE;
	device->registers->ci|=1<<cmd_slot;
}



static void KERNEL_CORE_CODE _device_wait_command(const ahci_device_t* device,u8 cmd_slot){
	while (device->registers->ci&(1<<cmd_slot)){
		asm volatile("pause");
	}
	device->registers->cmd&=~CMD_ST;
	while (device->registers->cmd&CMD_ST){
		asm volatile("pause");
	}
	device->registers->cmd&=~CMD_FRE;
}



static u64 KERNEL_CORE_CODE _ahci_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	ahci_device_t* device=extra_data;
	u32 dbc=(count<<9)-1;
	if (dbc>0x3fffff){
		dbc=0x3fffff;
	}
	u64 aligned_buffer_raw=0;
	u8* aligned_buffer=NULL;
	_Bool alignment_required=!!(((u64)buffer)&(PAGE_SIZE-1));
	if (alignment_required){
		aligned_buffer_raw=pmm_alloc((dbc+1)>>9);
		aligned_buffer=VMM_TRANSLATE_ADDRESS(aligned_buffer_raw);
		if (offset&DRIVE_OFFSET_FLAG_WRITE){
			memcpy(aligned_buffer,buffer,dbc+1);
		}
	}
	else{
		aligned_buffer_raw=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)buffer);
	}
	u8 cmd_slot=_device_get_command_slot(device);
	ahci_command_t* command=device->command_list->commands+cmd_slot;
	command->flags=(sizeof(ahci_fis_reg_h2d_t)>>2)|FLAGS_PREFEACHABLE;
	command->prdtl=1;
	ahci_command_table_t* command_table=device->command_tables[cmd_slot];
	command_table->prdt_entry->dba=aligned_buffer_raw;
	command_table->prdt_entry->dbau=aligned_buffer_raw>>32;
	command_table->prdt_entry->dbc=dbc;
	ahci_fis_reg_h2d_t* fis=(ahci_fis_reg_h2d_t*)(command_table->cfis);
	fis->fis_type=FIS_TYPE_REG_H2D;
	fis->flags=FIS_FLAG_COMMAND;
	fis->command=((offset&DRIVE_OFFSET_FLAG_WRITE)?ATA_CMD_WRITE:ATA_CMD_READ);
	fis->featurel=0;
	fis->lba0=offset;
	fis->lba1=offset>>8;
	fis->lba2=offset>>16;
	fis->device=0x40;
	fis->lba3=offset>>24;
	fis->lba4=offset>>32;
	fis->lba5=offset>>40;
	fis->featureh=0;
	fis->countl=count;
	fis->counth=count>>8;
	fis->icc=0;
	fis->control=0;
	_device_send_command(device,cmd_slot);
	_device_wait_command(device,cmd_slot);
	if (alignment_required){
		if (!(offset&DRIVE_OFFSET_FLAG_WRITE)){
			memcpy(buffer,aligned_buffer,dbc+1);
		}
		pmm_dealloc(aligned_buffer_raw,(dbc+1)>>9);
	}
	return (dbc+1)>>9;
}



static void KERNEL_CORE_CODE _ahci_init(ahci_device_t* device,u8 port_index){
	u64 command_list=pmm_alloc(1);
	device->registers->clb=command_list;
	device->registers->clbu=command_list>>32;
	device->command_list=VMM_TRANSLATE_ADDRESS(command_list);
	for (u8 i=0;i<32;i++){
		u64 command_table=pmm_alloc(1);
		device->command_tables[i]=VMM_TRANSLATE_ADDRESS(command_table);
		(device->command_list->commands+i)->ctba=command_table;
		(device->command_list->commands+i)->ctbau=command_table>>32;
	}
	u64 fis_base=pmm_alloc(1);
	device->registers->fb=fis_base;
	device->registers->fbu=fis_base>>32;
	device->registers->cmd|=CMD_ST|CMD_FRE;
	u8 cmd_slot=_device_get_command_slot(device);
	ahci_command_t* command=device->command_list->commands+cmd_slot;
	command->flags=(sizeof(ahci_fis_reg_h2d_t)>>2)|FLAGS_PREFEACHABLE;
	command->prdtl=1;
	u64 buffer_raw=pmm_alloc(1);
	ahci_command_table_t* command_table=device->command_tables[cmd_slot];
	command_table->prdt_entry->dba=buffer_raw;
	command_table->prdt_entry->dbau=buffer_raw>>32;
	command_table->prdt_entry->dbc=512;
	ahci_fis_reg_h2d_t* fis=(ahci_fis_reg_h2d_t*)(command_table->cfis);
	fis->fis_type=FIS_TYPE_REG_H2D;
	fis->flags=FIS_FLAG_COMMAND;
	fis->command=ATA_CMD_IDENTIFY;
	fis->featurel=0;
	fis->lba0=0;
	fis->lba1=0;
	fis->lba2=0;
	fis->device=0;
	fis->lba3=0;
	fis->lba4=0;
	fis->lba5=0;
	fis->featureh=0;
	fis->countl=0;
	fis->counth=0;
	fis->icc=0;
	fis->control=0;
	_device_send_command(device,cmd_slot);
	_device_wait_command(device,cmd_slot);
	const u8* buffer=VMM_TRANSLATE_ADDRESS(buffer_raw);
	drive_t drive={
		.type=DRIVE_TYPE_AHCI,
		.read_write=_ahci_read_write,
		.block_count=*((u64*)(buffer+200)),
		.block_size=512,
		.extra_data=device
	};
	drive.name[0]='a';
	drive.name[1]='h';
	drive.name[2]='c';
	drive.name[3]='i';
	if (port_index<10){
		drive.name[4]=port_index+48;
		drive.name[5]=0;
	}
	else{
		drive.name[4]=port_index/10+48;
		drive.name[5]=(port_index%10)+48;
	}
	drive_change_byte_order_and_truncate_spaces((const u16*)(buffer+20),10,drive.serial_number);
	drive_change_byte_order_and_truncate_spaces((const u16*)(buffer+54),20,drive.model_number);
	drive_list_add_drive(&drive);
	pmm_dealloc(buffer_raw,1);
}



void KERNEL_CORE_CODE driver_ahci_init(void){
	_ahci_controller_count=0;
	_ahci_devices=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(MAX_DEVICE_COUNT*sizeof(ahci_device_t))));
	_ahci_device_count=0;
}



void KERNEL_CORE_CODE driver_ahci_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x06||device->progif!=0x01){
		return;
	}
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,5,&pci_bar)){
		return;
	}
	LOG_CORE("Attached AHCI driver to PCI device %x:%x:%x",device->bus,device->slot,device->func);
	if (_ahci_controller_count>=MAX_CONTROLLER_COUNT){
		ERROR_CORE("Too many AHCI controllers");
		return;
	}
	ahci_controller_t* controller=_ahci_controllers+_ahci_controller_count;
	_ahci_controller_count++;
	controller->registers=VMM_TRANSLATE_ADDRESS(pci_bar.address);
	INFO_CORE("AHCI controller version: %x.%x",controller->registers->vs>>16,controller->registers->vs&0xffff);
	if (!(controller->registers->cap&CAP_S64A)){
		ERROR_CORE("AHCI controller does not support 64-bit addressing");
		return;
	}
	if (controller->registers->cap2&CAP2_BOH){
		controller->registers->bohc|=BOHC_OOS;
		while (controller->registers->bohc&(BOHC_BOS|BOHC_BB)){
			asm volatile("pause");
		}
		if ((controller->registers->bohc&BOHC_BB)||(controller->registers->bohc&BOHC_BOS)||!(controller->registers->bohc&BOHC_OOS)){
			ERROR_CORE("AHCI controller bios handoff failed");
			return;
		}
	}
	controller->registers->ghc=(controller->registers->ghc&(~GHC_IE))|GHC_AE;
	controller->port_count=controller->registers->cap&CAP_NP;
	controller->command_slot_count=(controller->registers->cap&CAP_NCS)>>CAP_NCS_SHIFT;
	for (u8 i=0;i<controller->port_count;i++){
		if (!(controller->registers->pi&(1<<i))){
			continue;
		}
		ahci_port_registers_t* port_registers=controller->registers->ports+i;
		if (port_registers->sig!=0x00000101){
			continue;
		}
		if (_ahci_device_count>=MAX_DEVICE_COUNT){
			ERROR_CORE("Too many AHCI devices");
			return;
		}
		ahci_device_t* ahci_device=_ahci_devices+_ahci_device_count;
		_ahci_device_count++;
		ahci_device->controller=controller;
		ahci_device->registers=port_registers;
		_ahci_init(ahci_device,i);
	}
}
