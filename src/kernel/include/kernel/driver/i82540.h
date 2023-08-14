#ifndef _KERNEL_DRIVER_I82540_H_
#define _KERNEL_DRIVER_I82540_H_
#include <kernel/pci/pci.h>
#include <kernel/types.h>



#define NUM_RX_DESCRIPTORS 512
#define NUM_TX_DESCRIPTORS 512



typedef volatile struct __attribute__((packed)) _I82540_RX_DESCRIPTOR{
	u64 address;
	u16 length;
	u8 _padding[2];
	u8 status;
	u8 errors;
	u8 _padding2[2];
} i82540_rx_descriptor_t;



typedef volatile struct __attribute__((packed)) _I82540_TX_DESCRIPTOR{
	u64 address;
	u16 length;
	u8 _padding[1];
	u8 cmd;
	u8 status;
	u8 _padding2[3];
} i82540_tx_descriptor_t;



typedef struct _I82540_DEVICE{
	volatile u32* mmio;
	u64 rx_desc_base;
	u64 tx_desc_base;
	u8 pci_irq;
	u8 irq;
} i82540_device_t;



void driver_i82540_init_device(pci_device_t* device);



#endif
