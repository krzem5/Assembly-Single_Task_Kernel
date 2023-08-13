#ifndef _KERNEL_DRIVER_ATA_H_
#define _KERNEL_DRIVER_ATA_H_
#include <kernel/pci/pci.h>



typedef struct _ATA_DEVICE{
	_Bool is_slave;
	_Bool is_atapi;
	u16 port;
	u64 dma_address;
} ata_device_t;



void driver_ata_init_device(pci_device_t* device);



#endif
