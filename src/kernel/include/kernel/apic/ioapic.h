#ifndef _KERNEL_APIC_IOAPIC_H_
#define _KERNEL_APIC_IOAPIC_H_ 1
#include <kernel/types.h>



void ioapic_init(void);



void ioapic_add(u8 apic_id,u32 address,u32 gsi_base);



#endif
