#include <kernel/apic/lapic.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lapic"



#define REGISTER_EOI 0x2c
#define REGISTER_SVR 0x3c
#define REGISTER_ESR 0xa0
#define REGISTER_ICR0 0xc0
#define REGISTER_ICR1 0xc4



volatile u32* _lapic_registers;



void lapic_init(u64 base){
	LOG("Initializing lAPIC controller...");
	INFO("lAPIC base: %p",base);
	_lapic_registers=VMM_TRANSLATE_ADDRESS(base);
}



void lapic_send_ipi(u8 lapic_id,u16 vector){
	_lapic_registers[REGISTER_ESR]=0;
	_lapic_registers[REGISTER_ICR1]=lapic_id<<24;
	_lapic_registers[REGISTER_ICR0]=vector;
	while (_lapic_registers[REGISTER_ICR0]&APIC_ICR0_DELIVERY_STATUS_PENDING){
		__pause();
	}
}



void lapic_enable(void){
	_lapic_registers[REGISTER_SVR]=0x1ff;
}
