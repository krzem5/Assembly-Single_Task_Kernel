#ifndef _AHCI_REGISTERS_H_
#define _AHCI_REGISTERS_H_ 1
#include <kernel/types.h>



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



#endif
