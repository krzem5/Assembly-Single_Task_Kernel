#ifndef _I82540_REGISTERS_H_
#define _I82540_REGISTERS_H_ 1
#include <kernel/types.h>



// Registers
#define REG_CTRL 0x0000
#define REG_STATUS 0x0002
#define REG_FCAL 0x000a
#define REG_FCAH 0x000b
#define REG_FCT 0x000c
#define REG_ICR 0x0030
#define REG_ITR 0x0031
#define REG_IMS 0x0034
#define REG_IMC 0x0036
#define REG_RCTL 0x0040
#define REG_FCTTV 0x005c
#define REG_TCTL 0x0100
#define REG_RDBAL 0x0a00
#define REG_RDBAH 0x0a01
#define REG_RDLEN 0x0a02
#define REG_RDH 0x0a04
#define REG_RDT 0x0a06
#define REG_TDBAL 0x0e00
#define REG_TDBAH 0x0e01
#define REG_TDLEN 0x0e02
#define REG_TDH 0x0e04
#define REG_TDT 0x0e06
#define REG_RAL0 0x1500
#define REG_RAH0 0x1501
#define REG_GCR 0x16c0

#define REG_MAX REG_GCR

// CTRL flags
#define CTRL_FD 0x00000001
#define CTRL_SLU 0x00000040
#define CTRL_RST 0x04000000

// RCTL flags
#define RCTL_EN 0x00000002
#define RCTL_SBP 0x00000004
#define RCTL_UPE 0x00000008
#define RCTL_MPE 0x00000010
#define RCTL_LPE 0x00000020
#define RCTL_BAM 0x00008000
#define RCTL_BSIZE_4096 0x02030000
#define RCTL_PMCF 0x00800000
#define RCTL_SECRC 0x04000000

// TCTL flags
#define TCTL_EN 0x02
#define TCTL_PSP 0x08

// RDESC status flafs
#define RDESC_DD 0x01
#define RDESC_EOP 0x02

// TXDESC command flags
#define TXDESC_EOP 0x01
#define TXDESC_RS 0x08

// TXDESC status flags
#define TXDESC_DD 0x01



typedef volatile struct KERNEL_PACKED _I82540_RX_DESCRIPTOR{
	u64 address;
	u16 length;
	u8 _padding[2];
	u8 status;
	u8 errors;
	u8 _padding2[2];
} i82540_rx_descriptor_t;



typedef volatile struct KERNEL_PACKED _I82540_TX_DESCRIPTOR{
	u64 address;
	u16 length;
	u8 _padding[1];
	u8 cmd;
	u8 status;
	u8 _padding2[3];
} i82540_tx_descriptor_t;



#endif
