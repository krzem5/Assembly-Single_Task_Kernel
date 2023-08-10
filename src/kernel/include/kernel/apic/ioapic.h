#ifndef _KERNEL_APIC_IOAPIC_H_
#define _KERNEL_APIC_IOAPIC_H_ 1
#include <kernel/types.h>



void ioapic_init(u16 count,u16 override_count);



void ioapic_add(u8 apic_id,u32 address,u32 gsi_base);



void ioapic_add_override(u8 irq,u32 gsi,u16 flags);



void ioapic_redirect_irq(u8 irq,u8 vector);



#endif
