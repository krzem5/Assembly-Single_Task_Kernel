#ifndef _KERNEL_APIC_LAPIC_H_
#define _KERNEL_APIC_LAPIC_H_ 1
#include <kernel/types.h>



#define APIC_ICR0_DELIVERY_MODE_INIT 0x0500
#define APIC_ICR0_DELIVERY_MODE_STARTUP 0x0600
#define APIC_ICR0_DELIVERY_STATUS_PENDING 0x1000
#define APIC_ICR0_LEVEL_ASSERT 0x4000
#define APIC_ICR0_TRIGGER_MODE_LEVEL 0x8000

#define LAPIC_SCHEDULER_VECTOR 0x20
#define LAPIC_WAKEUP_VECTOR 0xfe
#define LAPIC_SPURIOUS_VECTOR 0xff
#define LAPIC_DISABLE_TIMER 0x10000



void lapic_init(u64 base,u16 cpu_count);



void lapic_send_ipi(u8 lapic_id,u16 vector);



void lapic_eoi(void);



void lapic_enable(void);



void lapic_timer_start(u32 time_us);



u32 lapic_timer_stop(void);



#endif
