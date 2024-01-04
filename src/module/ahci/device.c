#include <ahci/device.h>
#include <ahci/registers.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "ahci"



static pmm_counter_descriptor_t* _ahci_driver_pmm_counter=NULL;
static omm_allocator_t* _ahci_controller_allocator=NULL;
static omm_allocator_t* _ahci_device_allocator=NULL;



static u16 _ahci_controller_index=0;



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



static u64 _ahci_read_write(drive_t* drive,u64 offset,u64 buffer,u64 count){
	ahci_device_t* device=drive->extra_data;
	u32 dbc=(count<<9)-1;
	if (dbc>0x3fffff){
		dbc=0x3fffff;
	}
	u8 cmd_slot=_device_get_command_slot(device);
	ahci_command_t* command=device->command_list->commands+cmd_slot;
	command->flags=(sizeof(ahci_fis_reg_h2d_t)>>2)|FLAGS_PREFEACHABLE;
	command->prdtl=1;
	ahci_command_table_t* command_table=device->command_tables[cmd_slot];
	command_table->prdt_entry->dba=buffer;
	command_table->prdt_entry->dbau=buffer>>32;
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
	return (dbc+1)>>9;
}



static const drive_type_t _ahci_drive_type_config={
	"ahci",
	0,
	_ahci_read_write
};



static void _ahci_init(ahci_device_t* device,u8 port_index){
	u64 command_list=pmm_alloc(1,_ahci_driver_pmm_counter,0);
	device->command_list=(void*)(command_list+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	device->registers->clb=command_list;
	device->registers->clbu=command_list>>32;
	for (u8 i=0;i<32;i++){
		u64 command_table=pmm_alloc(1,_ahci_driver_pmm_counter,0);
		device->command_tables[i]=(void*)(command_table+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		(device->command_list->commands+i)->ctba=command_table;
		(device->command_list->commands+i)->ctbau=command_table>>32;
	}
	u64 fis_base=pmm_alloc(1,_ahci_driver_pmm_counter,0);
	device->registers->fb=fis_base;
	device->registers->fbu=fis_base>>32;
	device->registers->cmd|=CMD_ST|CMD_FRE;
	u8 cmd_slot=_device_get_command_slot(device);
	ahci_command_t* command=device->command_list->commands+cmd_slot;
	command->flags=(sizeof(ahci_fis_reg_h2d_t)>>2)|FLAGS_PREFEACHABLE;
	command->prdtl=1;
	u64 buffer=pmm_alloc(1,_ahci_driver_pmm_counter,0);
	ahci_command_table_t* command_table=device->command_tables[cmd_slot];
	command_table->prdt_entry->dba=buffer;
	command_table->prdt_entry->dbau=buffer>>32;
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
	char serial_number_buffer[21];
	char model_number_buffer[41];
	memcpy_bswap16_trunc_spaces((const u16*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET+20),10,serial_number_buffer);
	memcpy_bswap16_trunc_spaces((const u16*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET+54),20,model_number_buffer);
	drive_config_t config={
		&_ahci_drive_type_config,
		device->controller->index,
		port_index,
		smm_alloc(serial_number_buffer,0),
		smm_alloc(model_number_buffer,0),
		*((u64*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET+200)),
		512,
		device
	};
	pmm_dealloc(buffer,1,_ahci_driver_pmm_counter);
	drive_create(&config);
}



static void _ahci_init_device(pci_device_t* device){
	if (device->class!=0x01||device->subclass!=0x06||device->progif!=0x01){
		return;
	}
	pci_device_enable_bus_mastering(device);
	pci_bar_t pci_bar;
	if (!pci_device_get_bar(device,5,&pci_bar)){
		return;
	}
	LOG("Attached AHCI driver to PCI device %x:%x:%x",device->address.bus,device->address.slot,device->address.func);
	ahci_controller_t* controller=omm_alloc(_ahci_controller_allocator);
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
			ERROR("AHCI controller BIOS handoff failed");
			return;
		}
	}
	controller->registers->ghc=(controller->registers->ghc&(~GHC_IE))|GHC_AE;
	controller->port_count=controller->registers->cap&CAP_NP;
	controller->command_slot_count=(controller->registers->cap&CAP_NCS)>>CAP_NCS_SHIFT;
	controller->index=_ahci_controller_index;
	_ahci_controller_index++;
	for (u8 i=0;i<controller->port_count;i++){
		if (!(controller->registers->pi&(1<<i))){
			continue;
		}
		ahci_port_registers_t* port_registers=controller->registers->ports+i;
		if (port_registers->sig!=0x00000101){
			continue;
		}
		ahci_device_t* ahci_device=omm_alloc(_ahci_device_allocator);
		ahci_device->controller=controller;
		ahci_device->registers=port_registers;
		_ahci_init(ahci_device,i);
	}
}



void ahci_locate_devices(void){
	_ahci_driver_pmm_counter=pmm_alloc_counter("ahci");
	_ahci_controller_allocator=omm_init("ahci_controller",sizeof(ahci_controller_t),8,1,pmm_alloc_counter("omm_ahci_controller"));
	spinlock_init(&(_ahci_controller_allocator->lock));
	_ahci_device_allocator=omm_init("ahci_device",sizeof(ahci_device_t),8,1,pmm_alloc_counter("omm_ahci_device"));
	spinlock_init(&(_ahci_device_allocator->lock));
	HANDLE_FOREACH(pci_device_handle_type){
		pci_device_t* device=handle->object;
		_ahci_init_device(device);
	}
}
