#ifndef _XHCI_REGISTERS_H_
#define _XHCI_REGISTERS_H_ 1
#include <kernel/types.h>



// Command Ring Control Register
#define CRCR_RCS 1

// USB Command
#define USBCMD_RS 1
#define USBCMD_HCRST 2

// USB Status
#define USBSTS_HCH 1
#define USBSTS_CNR 2048

// Port Status and Control Register
#define PORTSC_CCS 1
#define PORTSC_PLS_MASK 480
#define PORTSC_PLS_U0 0
#define PORTSC_PED 2
#define PORTSC_SPEED_MASK 15360
#define PORTSC_SPEED_FULL 1024
#define PORTSC_SPEED_LOW 2048
#define PORTSC_SPEED_HIGH 3072
#define PORTSC_SPEED_SUPER 4096

// Transfer Buffer
#define TRB_LK_TC 0x02
#define TRB_IOC 0x20
#define TRB_IDT 0x40

// Transfer Buffer Types
#define TRB_TYPE_MASK 0xfc00
#define TRB_TYPE_TR_SETUP 0x0800
#define TRB_TYPE_TR_DATA 0x0c00
#define TRB_TYPE_TR_STATUS 0x1000
#define TRB_TYPE_TR_LINK 0x1800
#define TRB_TYPE_CR_ENABLE_SLOT 0x2400
#define TRB_TYPE_CR_ADDRESS_DEVICE 0x2c00
#define TRB_TYPE_CR_CONFIGURE_ENDPOINT 0x3000
#define TRB_TYPE_CR_EVALUATE_CONTEXT 0x3400
#define TRB_TYPE_ER_TRANSFER 0x8000
#define TRB_TYPE_ER_COMMAND_COMPLETE 0x8400
#define TRB_TYPE_ER_PORT_STATUS_CHANGE 0x8800



typedef volatile struct _XHCI_REGISTERS{
	u8 caplength;
	u8 _padding;
	u16 hciversion;
	u32 hcsparams1;
	u32 hcsparams2;
	u32 hcsparams3;
	u32 hccparams1;
	u32 dboff;
	u32 rtsoff;
	u32 hccparams2;
} xhci_registers_t;



typedef volatile struct _XHCI_OPERATIONAL_REGISTERS{
	u32 usbcmd;
	u32 usbsts;
	u32 pagesize;
	u8 _padding[8];
	u32 dnctrl;
	u64 crcr;
	u8 _padding2[16];
	u64 dcbaap;
	u32 config;
} xhci_operational_registers_t;



typedef volatile struct _XHCI_PORT_REGISTERS{
	u32 portsc;
	u32 portpmsc;
	u32 portli;
	u8 _padding[4];
} xhci_port_registers_t;



typedef volatile struct _XHCI_DOORBELL{
	u32 value;
} xhci_doorbell_t;



typedef volatile struct _XHCI_INTERRUPT_REGISTERS{
	u32 iman;
	u32 imod;
	u32 erstsz;
	u8 _padding[4];
	u64 erstba;
	u64 erdp;
} xhci_interrupt_registers_t;



typedef volatile struct _XHCI_DEVICE_CONTEXT_BASE{
	u64 address;
} xhci_device_context_base_t;



typedef volatile struct _XHCI_TRANSFER_BLOCK{
	union{
		u64 address;
		u8 inline_data[8];
	};
	u32 status;
	u32 flags;
} xhci_transfer_block_t;



typedef volatile struct _XHCI_EVENT_RING_SEGMENT{
	u64 address;
	u32 size;
	u8 _padding[4];
} xhci_event_ring_segment_t;



typedef volatile struct _XHCI_SLOT_CONTEXT{
	u32 context[4];
	u8 _padding[16];
} xhci_slot_context_t;



typedef volatile struct _XHCI_ENDPOINT_CONTEXT{
	u32 context[2];
	u64 deq;
	u32 length;
	u8 _padding[12];
} xhci_ednpoint_context_t;



typedef volatile struct _XHCI_INPUT_CONTEXT{
	union{
		u8 _size[32];
		struct{
			u32 del;
			u32 address;
		} input;
		struct{
			u32 ctx[4];
		} slot;
		struct{
			u32 ctx[2];
			u64 deq;
			u32 length;
		} endpoint;
	};
} xhci_input_context_t;



#endif
