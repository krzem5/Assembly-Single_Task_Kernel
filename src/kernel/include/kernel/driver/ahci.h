#ifndef _KERNEL_DRIVER_AHCI_H_
#define _KERNEL_DRIVER_AHCI_H_
#include <kernel/types.h>



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



void driver_ahci_init(void);



#endif
