#include <kernel/apic/ioapic.h>
#include <kernel/io/io.h>
#include <kernel/isr/isr.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "serial"



#define COM1_IRQ 4



static lock_t _serial_read_lock=LOCK_INIT_STRUCT;
static lock_t _serial_write_lock=LOCK_INIT_STRUCT;
static u8 _serial_irq=0;



void serial_init_irq(void){
	LOG("Enabling serial IRQ...");
	_serial_irq=isr_allocate();
	ioapic_redirect_irq(COM1_IRQ,_serial_irq);
	io_port_out8(0x3f9,0x01);
	io_port_out8(0x3fc,0x0b);
}



void KERNEL_CORE_CODE serial_send(const void* buffer,u32 length){
	lock_acquire(&_serial_read_lock);
	for (;length;length--){
		while (!(io_port_in8(0x3fd)&0x20)){
			__pause();
		}
		io_port_out8(0x3f8,*((const u8*)buffer));
		buffer++;
	}
	lock_release(&_serial_read_lock);
}



u32 serial_recv(void* buffer,u32 length,u64 timeout){
	lock_acquire(&_serial_write_lock);
	u32 out=0;
	if (!timeout){
		for (;out<length;out++){
			while (!(io_port_in8(0x3fd)&0x01)){
				if (!isr_wait(_serial_irq)){
					return 0;
				}
			}
			*((u8*)buffer)=io_port_in8(0x3f8);
			buffer++;
		}
	}
	else{
		for (;out<length;out++){
			u64 i=timeout-1;
			do{
				__pause();
				i--;
			} while (i&&!(io_port_in8(0x3fd)&0x01));
			if (!i){
				break;
			}
			*((u8*)buffer)=io_port_in8(0x3f8);
			buffer++;
		}
	}
	lock_release(&_serial_write_lock);
	return out;
}
