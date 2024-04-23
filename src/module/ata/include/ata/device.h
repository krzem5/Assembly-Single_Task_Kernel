#ifndef _ATA_DEVICE_H_
#define _ATA_DEVICE_H_ 1
#include <kernel/types.h>



typedef struct _ATA_DEVICE{
	bool is_slave;
	bool is_atapi;
	u16 port;
	void* dma_address;
} ata_device_t;



#endif
