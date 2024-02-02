#ifndef _I82540_DEVICE_H_
#define _I82540_DEVICE_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/mp/event.h>
#include <kernel/types.h>



#define I82540_DEVICE_GET_DESCRIPTOR(device,type,index) ((void*)((device)->type##_desc_base+((index)<<4)))



typedef struct _I82540_DEVICE{
	spinlock_t lock;
	volatile u32* mmio;
	u64 rx_desc_base;
	u64 tx_desc_base;
	u8 irq;
	event_t* irq_event;
} i82540_device_t;



void i82540_locate_devices(void);



#endif
