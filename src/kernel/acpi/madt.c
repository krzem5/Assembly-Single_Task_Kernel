#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "madt"



typedef struct __attribute__((packed)) _MADT{
	u32 signature;
	u32 length;
	u8 revision;
	u8 _padding[27];
	u32 lapic;
	u8 _padding2[4];
	u8 entries[];
} madt_t;



typedef struct __attribute__((packed)) _MADT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct __attribute__((packed)){
			u8 acpi_processor_id;
			u8 apic_id;
			u32 flags;
		} lapic;
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
	u64 lapic_address=madt->lapic;
	for (u32 i=0;i<madt->length-sizeof(madt_t);){
		const madt_entry_t* madt_entry=(const madt_entry_t*)(madt->entries+i);
		if (!madt_entry->type){
			if (!(madt_entry->lapic.flags&1)){
				WARN("CPU#%u not yet online!",madt_entry->lapic.acpi_processor_id);
			}
			else{
				cpu_count++;
			}
		}
		else if (madt_entry->type==5){
			lapic_address=madt_entry->lapic_override.lapic;
		}
		i+=madt_entry->length;
	}
	cpu_init(cpu_count,lapic_address);
	for (u32 i=0;i<madt->length-sizeof(madt_t);){
		const madt_entry_t* madt_entry=(const madt_entry_t*)(madt->entries+i);
		if (!madt_entry->type&&(madt_entry->lapic.flags&1)){
			cpu_register_core(madt_entry->lapic.acpi_processor_id,madt_entry->lapic.apic_id);
		}
		i+=madt_entry->length;
	}
}
