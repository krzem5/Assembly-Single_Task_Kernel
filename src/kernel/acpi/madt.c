#include <kernel/acpi/structures.h>
#include <kernel/apic/ioapic.h>
#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "madt"



void acpi_madt_load(const acpi_madt_t* madt){
	LOG("Loading MADT...");
	u16 cpu_count=0;
	u16 ioapic_count=0;
	u16 iso_count=0;
	u64 lapic_address=madt->lapic;
	for (u32 i=0;i<madt->header.length-sizeof(acpi_madt_t);){
		const acpi_madt_entry_t* madt_entry=(const acpi_madt_entry_t*)(madt->entries+i);
		if (madt_entry->type==ACPI_MADT_ENTRY_TYPE_LAPIC){
			if (madt_entry->lapic.processor_id!=madt_entry->lapic.lapic_id){
				WARN("CPU code id does not match CPU APIC id");
			}
			else if (!(madt_entry->lapic.flags&1)){
				WARN("CPU#%u not yet online",madt_entry->lapic.processor_id);
			}
			else{
				cpu_count++;
			}
		}
		else if (madt_entry->type==ACPI_MADT_ENTRY_TYPE_IOAPIC){
			ioapic_count++;
		}
		else if (madt_entry->type==ACPI_MADT_ENTRY_TYPE_ISO){
			iso_count++;
		}
		else if (madt_entry->type==ACPI_MADT_ENTRY_TYPE_LAPIC_OVERRIDE){
			lapic_address=madt_entry->lapic_override.lapic;
		}
		i+=madt_entry->length;
	}
	lapic_init(lapic_address,cpu_count);
	ioapic_init(ioapic_count,iso_count);
	cpu_init(cpu_count);
	for (u32 i=0;i<madt->header.length-sizeof(acpi_madt_t);){
		const acpi_madt_entry_t* madt_entry=(const acpi_madt_entry_t*)(madt->entries+i);
		if (madt_entry->type==ACPI_MADT_ENTRY_TYPE_IOAPIC){
			ioapic_add(madt_entry->io_apic.apic_id,madt_entry->io_apic.address,madt_entry->io_apic.gsi_base);
		}
		else if (madt_entry->type==ACPI_MADT_ENTRY_TYPE_ISO){
			ioapic_add_override(madt_entry->iso.irq,madt_entry->iso.gsi,madt_entry->iso.flags);
		}
		i+=madt_entry->length;
	}
}
