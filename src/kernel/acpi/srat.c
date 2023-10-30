#include <kernel/log/log.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "srat"



#define SRAT_ENTRY_TYPE_PROCESSOR 0
#define SRAT_ENTRY_TYPE_MEMORY 1

#define SRAT_ENTRY_PROCESSOR_FLAG_PRESENT 1
#define SRAT_ENTRY_MEMORY_FLAG_PRESENT 1



typedef struct __attribute__((packed)) _SRAT{
	u32 signature;
	u32 length;
	u8 _padding[40];
	u8 data[];
} srat_t;



typedef struct __attribute__((packed)) _SRAT_ENTRY{
	u8 type;
	u8 length;
	union{
		struct __attribute__((packed)){
			u8 proximity_domain_low;
			u8 apic_id;
			u32 flags;
			u8 sapic_eid;
			u8 proximity_domain_high[3];
			u32 clock_domain;
		} processor;
		struct __attribute__((packed)){
			u32 proximity_domain;
			u8 _padding[2];
			u64 base_address;
			u64 length;
			u8 _padding2[4];
			u32 flags;
		} memory;
	};
} srat_entry_t;



void acpi_srat_load(const void* srat_ptr){
	LOG("Loading SRAT...");
	const srat_t* srat=srat_ptr;
	u32 max_proximity_domain=0;
	u32 numa_cpu_count=0;
	u32 numa_memory_range_count=0;
	for (u32 offset=0;offset<srat->length-sizeof(srat_t);){
		const srat_entry_t* entry=(const srat_entry_t*)(srat->data+offset);
		u32 proximity_domain=0;
		if (entry->type==SRAT_ENTRY_TYPE_PROCESSOR&&(entry->processor.flags&SRAT_ENTRY_PROCESSOR_FLAG_PRESENT)){
			proximity_domain=(entry->processor.proximity_domain_high[0]<<24)|(entry->processor.proximity_domain_high[1]<<16)|(entry->processor.proximity_domain_high[2]<<8)|entry->processor.proximity_domain_low;
			numa_cpu_count++;
		}
		else if (entry->type==SRAT_ENTRY_TYPE_MEMORY&&(entry->memory.flags&SRAT_ENTRY_MEMORY_FLAG_PRESENT)){
			proximity_domain=entry->memory.proximity_domain;
			numa_memory_range_count++;
		}
		if (proximity_domain>max_proximity_domain){
			max_proximity_domain=proximity_domain;
		}
		offset+=entry->length;
	}
	numa_init(max_proximity_domain+1,numa_cpu_count,numa_memory_range_count);
	for (u32 offset=0;offset<srat->length-sizeof(srat_t);){
		const srat_entry_t* entry=(const srat_entry_t*)(srat->data+offset);
		if (entry->type==SRAT_ENTRY_TYPE_PROCESSOR&&(entry->processor.flags&SRAT_ENTRY_PROCESSOR_FLAG_PRESENT)){
			u32 proximity_domain=(entry->processor.proximity_domain_high[0]<<24)|(entry->processor.proximity_domain_high[1]<<16)|(entry->processor.proximity_domain_high[2]<<8)|entry->processor.proximity_domain_low;
			numa_add_cpu(proximity_domain,entry->processor.apic_id,entry->processor.sapic_eid);
		}
		else if (entry->type==SRAT_ENTRY_TYPE_MEMORY&&(entry->memory.flags&SRAT_ENTRY_MEMORY_FLAG_PRESENT)){
			numa_add_memory_range(entry->memory.proximity_domain,entry->memory.base_address,entry->memory.base_address+entry->memory.length,!!(entry->memory.flags&2));
		}
		offset+=entry->length;
	}
}
