#ifndef _KERNEL_DRIVER_ATA_H_
#define _KERNEL_DRIVER_ATA_H_
#include <kernel/types.h>



typedef struct _ATA_DEVICE{
	_Bool is_slave;
	_Bool is_atapi;
	u16 port;
	void* dma_address;
} ata_device_t;



void driver_ata_init(void);



#endif
