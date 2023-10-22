#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "ahci"



#define PMM_COUNTER_DRIVER_AHCI (0x10000|HANDLE_TYPE_PMM_COUNTER)// PMM_DECLARE_COUNTER(DRIVER_AHCI);



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



typedef volatile struct _AHCI_PORT_REGISTERS{
	u32 clb;
	u32 clbu;
	u32 fb;
	u32 fbu;
	u32 is;
	u32 ie;
	u32 cmd;
	u32 rsv0;
	u32 tfd;
	u32 sig;
	u32 ssts;
	u32 sctl;
	u32 serr;
	u32 sact;
	u32 ci;
	u32 sntf;
	u32 fbs;
	u8 _padding[60];
} ahci_port_registers_t;



typedef volatile struct _AHCI_REGISTERS{
	u32 cap;
	u32 ghc;
	u32 is;
	u32 pi;
	u32 vs;
	u32 ccc_ctl;
	u32 ccc_pts;
	u32 em_loc;
	u32 em_ctl;
	u32 cap2;
	u32 bohc;
	u8 _padding[212];
	ahci_port_registers_t ports[32];
} ahci_registers_t;



typedef volatile struct _AHCI_COMMAND{
	u16 flags;
	u16 prdtl;
	u32 prdbc;
	u32 ctba;
	u32 ctbau;
	u8 _padding[16];
} ahci_command_t;



typedef volatile struct _AHCI_COMMAND_LIST{
	ahci_command_t commands[32];
} ahci_command_list_t;



typedef volatile struct _AHCI_PHYSICAL_REGION_DESCRIPTOR_TABLE{
	u32 dba;
	u32 dbau;
	u8 _padding[4];
	u32 dbc;
} ahci_physical_region_descriptor_table_t;



typedef volatile struct _AHCI_COMMAND_TABLE{
	u8 cfis[64];
	u8 acmd[16];
	u8 _padding[48];
	ahci_physical_region_descriptor_table_t prdt_entry[1];
} ahci_command_table_t;



typedef volatile struct _AHCI_FIS_REG_H2D{
	u8 fis_type;
	u8 flags;
	u8 command;
	u8 featurel;
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;
	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 featureh;
	u8 countl;
	u8 counth;
	u8 icc;
	u8 control;
	u8 _padding[4];
} ahci_fis_reg_h2d_t;



typedef volatile struct _AHCI_FIS_REG_D2H{
	u8 fis_type;
	u8 flags;
	u8 status;
	u8 error;
	u8 lba0;
	u8 lba1;
	u8 lba2;
	u8 device;
	u8 lba3;
	u8 lba4;
	u8 lba5;
	u8 _padding;
	u8 countl;
	u8 counth;
	u8 _padding2[2];
} ahci_fis_reg_d2h_t;



typedef struct _AHCI_CONTROLLER{
	ahci_registers_t* registers;
	u8 port_count;
	u8 command_slot_count;
} ahci_controller_t;



typedef struct _AHCI_DEVICE{
	ahci_controller_t* controller;
	ahci_port_registers_t* registers;
	ahci_command_list_t* command_list;
	ahci_command_table_t* command_tables[32];
} ahci_device_t;



static u8 _device_get_command_slot(const ahci_device_t* device){
	u32 mask;
	do{
		mask=(~(device->registers->sact|device->registers->ci))&((1ull<<device->controller->command_slot_count)-1);
	} while (!mask);
	return __builtin_ctz(mask);
}



static void _device_send_command(const ahci_device_t* device,u8 cmd_slot){
	SPINLOOP(device->registers->tfd&(TFD_STS_DSQ|TFD_STS_BSY));
	device->registers->cmd&=~CMD_ST;
	SPINLOOP(device->registers->cmd&CMD_CR);
	device->registers->cmd|=CMD_ST|CMD_FRE;
	device->registers->ci|=1<<cmd_slot;
}



static void _device_wait_command(const ahci_device_t* device,u8 cmd_slot){
	SPINLOOP(device->registers->ci&(1<<cmd_slot));
	device->registers->cmd&=~CMD_ST;
	SPINLOOP(device->registers->cmd&CMD_ST);
	device->registers->cmd&=~CMD_FRE;
}



static u64 _ahci_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	ahci_device_t* device=extra_data;
	u32 dbc=(count<<9)-1;
	if (dbc>0x3fffff){
		dbc=0x3fffff;
	}
	u64 aligned_buffer=pmm_alloc((dbc+1)>>9,PMM_COUNTER_DRIVER_AHCI,0);
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		memcpy((void*)(aligned_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),buffer,dbc+1);
	}
	u8 cmd_slot=_device_get_command_slot(device);
	ahci_command_t* command=device->command_list->commands+cmd_slot;
	command->flags=(sizeof(ahci_fis_reg_h2d_t)>>2)|FLAGS_PREFEACHABLE;
	command->prdtl=1;
	ahci_command_table_t* command_table=device->command_tables[cmd_slot];
	command_table->prdt_entry->dba=aligned_buffer;
	command_table->prdt_entry->dbau=aligned_buffer>>32;
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
	if (!(offset&DRIVE_OFFSET_FLAG_WRITE)){
		memcpy(buffer,(void*)(aligned_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),dbc+1);
	}
	pmm_dealloc(aligned_buffer,(dbc+1)>>9,PMM_COUNTER_DRIVER_AHCI);
	return (dbc+1)>>9;
}



static drive_type_t _ahci_drive_type={
	"AHCI",
	_ahci_read_write
};



static void _ahci_init(ahci_device_t* device,u8 port_index){
	u64 command_list=pmm_alloc(1,PMM_COUNTER_DRIVER_AHCI,0);
	device->command_list=(void*)(command_list+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	device->registers->clb=command_list;
	device->registers->clbu=command_list>>32;
	for (u8 i=0;i<32;i++){
		u64 command_table=pmm_alloc(1,PMM_COUNTER_DRIVER_AHCI,0);
		device->command_tables[i]=(void*)(command_table+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		(device->command_list->commands+i)->ctba=command_table;
		(device->command_list->commands+i)->ctbau=command_table>>32;
	}
	u64 fis_base=pmm_alloc(1,PMM_COUNTER_DRIVER_AHCI,0);
	device->registers->fb=fis_base;
	device->registers->fbu=fis_base>>32;
	device->registers->cmd|=CMD_ST|CMD_FRE;
	u8 cmd_slot=_device_get_command_slot(device);
	ahci_command_t* command=device->command_list->commands+cmd_slot;
	command->flags=(sizeof(ahci_fis_reg_h2d_t)>>2)|FLAGS_PREFEACHABLE;
	command->prdtl=1;
	const u8* buffer=(void*)pmm_alloc(1,PMM_COUNTER_DRIVER_AHCI,0);
	ahci_command_table_t* command_table=device->command_tables[cmd_slot];
	command_table->prdt_entry->dba=((u64)buffer);
	command_table->prdt_entry->dbau=((u64)buffer)>>32;
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
	drive_config_t config={
		.type=&_ahci_drive_type,
		.block_count=*((u64*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET+200)),
		.block_size=512,
		.extra_data=device
	};
	format_string(config.name,DRIVE_NAME_LENGTH,"ahci%u",port_index);
	bswap16_trunc_spaces((const u16*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET+20),10,config.serial_number);
	bswap16_trunc_spaces((const u16*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET+54),20,config.model_number);
	drive_create(&config);
	pmm_dealloc((u64)buffer,1,PMM_COUNTER_DRIVER_AHCI);
}



void driver_ahci_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x06||device->progif!=0x01){
		return;
	}
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,5,&pci_bar)){
		return;
	}
	LOG("Attached AHCI driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	ahci_controller_t* controller=kmm_alloc(sizeof(ahci_controller_t));
	controller->registers=(void*)vmm_identity_map(pci_bar.address,sizeof(ahci_registers_t));
	INFO("AHCI controller version: %x.%x",controller->registers->vs>>16,controller->registers->vs&0xffff);
	if (!(controller->registers->cap&CAP_S64A)){
		ERROR("AHCI controller does not support 64-bit addressing");
		return;
	}
	if (controller->registers->cap2&CAP2_BOH){
		controller->registers->bohc|=BOHC_OOS;
		SPINLOOP(controller->registers->bohc&(BOHC_BOS|BOHC_BB));
		if ((controller->registers->bohc&BOHC_BB)||(controller->registers->bohc&BOHC_BOS)||!(controller->registers->bohc&BOHC_OOS)){
			ERROR("AHCI controller bios handoff failed");
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
		ahci_device_t* ahci_device=kmm_alloc(sizeof(ahci_device_t));
		ahci_device->controller=controller;
		ahci_device->registers=port_registers;
		_ahci_init(ahci_device,i);
	}
}



static _Bool _init(module_t* module){
	drive_register_type(&_ahci_drive_type);
	HANDLE_FOREACH(HANDLE_TYPE_PCI_DEVICE){
		pci_device_t* device=handle->object;
		driver_ahci_init_device(device);
	}
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"ahci",
	_init,
	_deinit
);
