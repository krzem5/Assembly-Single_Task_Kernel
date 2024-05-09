#ifndef _KERNEL_SERIAL_SERIAL_H_
#define _KERNEL_SERIAL_SERIAL_H_ 1
#include <kernel/lock/rwlock.h>
#include <kernel/types.h>



#define SERIAL_PORT_COUNT 4



typedef struct _SERIAL_PORT{
	rwlock_t read_lock;
	rwlock_t write_lock;
	u16 io_port;
} serial_port_t;



extern serial_port_t serial_ports[SERIAL_PORT_COUNT];
extern serial_port_t* serial_default_port;



void serial_init(void);



void serial_send(serial_port_t* port,const void* buffer,u32 length);



u32 serial_recv(serial_port_t* port,void* buffer,u32 length);



#endif
