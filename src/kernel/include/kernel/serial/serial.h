#ifndef _KERNEL_SERIAL_SERIAL_H_
#define _KERNEL_SERIAL_SERIAL_H_ 1
#include <kernel/types.h>



void serial_init(void);



void serial_init_irq(void);



void serial_send(const void* buffer,u32 length);



u32 serial_recv(void* buffer,u32 length,u64 timeout);



#endif
