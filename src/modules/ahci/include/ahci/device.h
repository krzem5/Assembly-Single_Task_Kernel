#ifndef _AHCI_DEVICE_H_
#define _AHCI_DEVICE_H_ 1
#include <ahci/registers.h>
#include <kernel/types.h>



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



void ahci_locate_devices(void);



#endif
