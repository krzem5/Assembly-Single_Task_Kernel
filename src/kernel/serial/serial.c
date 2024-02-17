#include <kernel/apic/ioapic.h>
#include <kernel/io/io.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/serial/serial.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "serial"



#define COM1_3_IRQ 4
#define COM2_4_IRQ 3



static u8 KERNEL_INIT_WRITE _serial_irq=0;
static event_t* KERNEL_INIT_WRITE _serial_irq_event=NULL;

KERNEL_PUBLIC serial_port_t __attribute__((section(".data"))) serial_ports[SERIAL_PORT_COUNT]; // not defined as KERNEL_INIT_WRITE due to inline spinlocks
KERNEL_PUBLIC serial_port_t* KERNEL_INIT_WRITE serial_default_port=NULL;



static void KERNEL_EARLY_EXEC _init_port(u16 io_port,serial_port_t* out){
	out->io_port=0;
	io_port_out8(io_port+1,0x00);
	io_port_out8(io_port+3,0x80);
	io_port_out8(io_port,0x03);
	io_port_out8(io_port+1,0x00);
	io_port_out8(io_port+3,0x03);
	io_port_out8(io_port+2,0xc7);
	io_port_out8(io_port+4,0x1e);
	io_port_out8(io_port,0xa5);
	if (io_port_in8(io_port)!=0xa5){
		return;
	}
	io_port_out8(io_port+4,0x03);
	spinlock_init(&(out->read_lock));
	spinlock_init(&(out->write_lock));
	out->io_port=io_port;
	if (!serial_default_port){
		serial_default_port=out;
	}
}



static void _serial_irq_handler(void* ctx){
	event_dispatch(_serial_irq_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_BYPASS_ACL);
}



KERNEL_INIT(){
	LOG("Enabling serial IRQs...");
	_serial_irq=isr_allocate();
	INFO("Serial IRQ: %u",_serial_irq);
	_serial_irq_event=event_create();
	ioapic_redirect_irq(COM1_3_IRQ,_serial_irq);
	ioapic_redirect_irq(COM2_4_IRQ,_serial_irq);
	IRQ_HANDLER_CTX(_serial_irq)=NULL;
	IRQ_HANDLER(_serial_irq)=_serial_irq_handler;
	for (u8 i=0;i<SERIAL_PORT_COUNT;i++){
		serial_port_t* port=serial_ports+i;
		if (!port->io_port){
			continue;
		}
		io_port_out8(port->io_port+1,0x01);
		io_port_out8(port->io_port+4,0x0b);
	}
}



void KERNEL_EARLY_EXEC serial_init(void){
	_init_port(0x3f8,serial_ports);
	_init_port(0x2f8,serial_ports+1);
	_init_port(0x3e8,serial_ports+2);
	_init_port(0x2e8,serial_ports+3);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE serial_send(serial_port_t* port,const void* buffer,u32 length){
	scheduler_pause();
	spinlock_acquire_exclusive(&(port->write_lock));
	for (;length;length--){
		SPINLOOP(!(io_port_in8(port->io_port+5)&0x20));
		io_port_out8(port->io_port,*((const u8*)buffer));
		buffer++;
	}
	spinlock_release_exclusive(&(port->write_lock));
	scheduler_resume();
}



KERNEL_PUBLIC u32 serial_recv(serial_port_t* port,void* buffer,u32 length){
	spinlock_acquire_exclusive(&(port->read_lock));
	for (u32 i=0;i<length;i++){
		while (!(io_port_in8(port->io_port+5)&0x01)){
			spinlock_release_exclusive(&(port->read_lock));
			event_await(_serial_irq_event,1);
			event_set_active(_serial_irq_event,0,0);
			spinlock_acquire_exclusive(&(port->read_lock));
		}
		*((u8*)buffer)=io_port_in8(port->io_port);
		buffer++;
	}
	spinlock_release_exclusive(&(port->read_lock));
	return length;
}
