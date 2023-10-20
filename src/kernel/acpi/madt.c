#include <kernel/apic/ioapic.h>
#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "madt"



#define MADT_ENTRY_TYPE_LAPIC 0
#define MADT_ENTRY_TYPE_IOAPIC 1
#define MADT_ENTRY_TYPE_ISO 2
#define MADT_ENTRY_TYPE_LAPIC_OVERRIDE 5



typedef struct __attribute__((packed)) _MADT{
	u32 signature;
	u32 length;
	u8 _padding[28];
	u32 lapic;
	u8 _padding2[4];
	u8 entries[];
} madt_t;



typedef struct __attribute__((packed)) _MADT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct __attribute__((packed)){
			u8 processor_id;
			u8 lapic_id;
			u32 flags;
		} lapic;
		struct __attribute__((packed)){
			u8 apic_id;
			u8 _padding;
			u32 address;
			u32 gsi_base;
		} io_apic;
		struct __attribute__((packed)){
			u8 _padding;
			u8 irq;
			u32 gsi;
			u16 flags;
		} iso;
		struct __attribute__((packed)){
			u8 _padding[2];
			u64 lapic;
		} lapic_override;
	};
} madt_entry_t;



void acpi_madt_load(const void* madt_ptr){
	LOG("Loading MADT...");
	const madt_t* madt=madt_ptr;
	u16 cpu_count=0;
	u16 ioapic_count=0;
	u16 iso_count=0;
	u64 lapic_address=madt->lapic;
	for (u32 i=0;i<madt->length-sizeof(madt_t);){
		const madt_entry_t* madt_entry=(const madt_entry_t*)(madt->entries+i);
		if (madt_entry->type==MADT_ENTRY_TYPE_LAPIC){
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
		else if (madt_entry->type==MADT_ENTRY_TYPE_IOAPIC){
			ioapic_count++;
		}
		else if (madt_entry->type==MADT_ENTRY_TYPE_ISO){
			iso_count++;
		}
		else if (madt_entry->type==MADT_ENTRY_TYPE_LAPIC_OVERRIDE){
			lapic_address=madt_entry->lapic_override.lapic;
		}
		i+=madt_entry->length;
	}
	lapic_init(lapic_address,cpu_count);
	ioapic_init(ioapic_count,iso_count);
	cpu_init(cpu_count);
	for (u32 i=0;i<madt->length-sizeof(madt_t);){
		const madt_entry_t* madt_entry=(const madt_entry_t*)(madt->entries+i);
		if (madt_entry->type==MADT_ENTRY_TYPE_IOAPIC){
			ioapic_add(madt_entry->io_apic.apic_id,madt_entry->io_apic.address,madt_entry->io_apic.gsi_base);
		}
		else if (madt_entry->type==MADT_ENTRY_TYPE_ISO){
			ioapic_add_override(madt_entry->iso.irq,madt_entry->iso.gsi,madt_entry->iso.flags);
		}
		i+=madt_entry->length;
	}
}
