#ifndef _KERNEL_APIC_LAPIC_H_
#define _KERNEL_APIC_LAPIC_H_ 1
#include <kernel/types.h>



#define APIC_ICR0_DELIVERY_MODE_INIT 0x0500
#define APIC_ICR0_DELIVERY_MODE_STARTUP 0x0600
#define APIC_ICR0_DELIVERY_STATUS_PENDING 0x1000
#define APIC_ICR0_LEVEL_ASSERT 0x4000
#define APIC_ICR0_TRIGGER_MODE_LEVEL 0x8000



void lapic_init(u64 base);



void lapic_send_ipi(u8 apic_id,u16 vector,_Bool wait_for_delivery);



void lapic_enable(void);



#endif
