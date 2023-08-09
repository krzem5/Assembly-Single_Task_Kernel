#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "ioapic"



#define REGISTER_VER 0x01



void ioapic_init(void){
	LOG("Initializing IOAPIC controller...");
}



void ioapic_add(u8 apic_id,u32 address,u32 gsi_base){
	//
}
