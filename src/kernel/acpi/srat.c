#include <kernel/acpi/structures.h>
#include <kernel/log/log.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "srat"



void KERNEL_EARLY_EXEC acpi_srat_load(const acpi_srat_t* srat){
	LOG("Loading SRAT...");
	u32 max_proximity_domain=0;
	u32 numa_cpu_count=0;
	u32 numa_memory_range_count=0;
	for (u32 offset=0;offset<srat->header.length-sizeof(acpi_srat_t);){
		const acpi_srat_entry_t* entry=(const acpi_srat_entry_t*)(srat->data+offset);
		u32 proximity_domain=0;
		if (entry->type==ACPI_SRAT_ENTRY_TYPE_PROCESSOR&&(entry->processor.flags&ACPI_SRAT_ENTRY_PROCESSOR_FLAG_PRESENT)){
			proximity_domain=(entry->processor.proximity_domain_high[0]<<24)|(entry->processor.proximity_domain_high[1]<<16)|(entry->processor.proximity_domain_high[2]<<8)|entry->processor.proximity_domain_low;
			numa_cpu_count++;
		}
		else if (entry->type==ACPI_SRAT_ENTRY_TYPE_MEMORY&&(entry->memory.flags&ACPI_SRAT_ENTRY_MEMORY_FLAG_PRESENT)){
			proximity_domain=entry->memory.proximity_domain;
			numa_memory_range_count++;
		}
		if (proximity_domain>max_proximity_domain){
			max_proximity_domain=proximity_domain;
		}
		offset+=entry->length;
	}
	numa_init(max_proximity_domain+1,numa_cpu_count,numa_memory_range_count);
	for (u32 offset=0;offset<srat->header.length-sizeof(acpi_srat_t);){
		const acpi_srat_entry_t* entry=(const acpi_srat_entry_t*)(srat->data+offset);
		if (entry->type==ACPI_SRAT_ENTRY_TYPE_PROCESSOR&&(entry->processor.flags&ACPI_SRAT_ENTRY_PROCESSOR_FLAG_PRESENT)){
			u32 proximity_domain=(entry->processor.proximity_domain_high[0]<<24)|(entry->processor.proximity_domain_high[1]<<16)|(entry->processor.proximity_domain_high[2]<<8)|entry->processor.proximity_domain_low;
			numa_add_cpu(proximity_domain,entry->processor.apic_id,entry->processor.sapic_eid);
		}
		else if (entry->type==ACPI_SRAT_ENTRY_TYPE_MEMORY&&(entry->memory.flags&ACPI_SRAT_ENTRY_MEMORY_FLAG_PRESENT)){
			numa_add_memory_range(entry->memory.proximity_domain,entry->memory.base_address,entry->memory.base_address+entry->memory.length,!!(entry->memory.flags&2));
		}
		offset+=entry->length;
	}
}
