#ifndef _KERNEL_SERIAL_SERIAL_H_
#define _KERNEL_SERIAL_SERIAL_H_ 1
#include <kernel/types.h>



void serial_send(const void* buffer,u32 length);



void serial_recv(void* buffer,u32 length);



#endif
