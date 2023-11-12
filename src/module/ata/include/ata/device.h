#ifndef _ATA_DEVICE_H_
#define _ATA_DEVICE_H_ 1
#include <kernel/types.h>



typedef struct _ATA_DEVICE{
	_Bool is_slave;
	_Bool is_atapi;
	u16 port;
	void* dma_address;
} ata_device_t;



void ata_locate_devices(void);



#endif
