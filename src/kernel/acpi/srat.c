#include <kernel/log/log.h>
#include <kernel/numa/numa.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "srat"



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
	for (u32 offset=0;offset<srat->length-sizeof(srat_t);){
		const srat_entry_t* entry=(const srat_entry_t*)(srat->data+offset);
		u32 proximity_domain=0;
		if (!entry->type&&(entry->processor.flags&1)){
			proximity_domain=(entry->processor.proximity_domain_high[0]<<24)|(entry->processor.proximity_domain_high[1]<<16)|(entry->processor.proximity_domain_high[2]<<8)|entry->processor.proximity_domain_low;
		}
		else if (entry->type==1&&(entry->memory.flags&1)){
			proximity_domain=entry->memory.proximity_domain;
		}
		if (proximity_domain>max_proximity_domain){
			max_proximity_domain=proximity_domain;
		}
		offset+=entry->length;
	}
	numa_init(max_proximity_domain+1);
	for (u32 offset=0;offset<srat->length-sizeof(srat_t);){
		const srat_entry_t* entry=(const srat_entry_t*)(srat->data+offset);
		if (!entry->type&&(entry->processor.flags&1)){
			u32 proximity_domain=(entry->processor.proximity_domain_high[0]<<24)|(entry->processor.proximity_domain_high[1]<<16)|(entry->processor.proximity_domain_high[2]<<8)|entry->processor.proximity_domain_low;
			INFO("CPU [%u] -> [%u] #%u",entry->processor.apic_id,entry->processor.sapic_eid,proximity_domain);
		}
		else if (entry->type==1&&(entry->memory.flags&1)){
			INFO("MEM %p - %p #%u%s",entry->memory.base_address,entry->memory.base_address+entry->memory.length,entry->memory.proximity_domain,((entry->memory.flags&2)?" Hot-pluggable":""));
		}
		else if (entry->type>1){
			WARN("Unknown SRAT table entry: %u",entry->type);
		}
		offset+=entry->length;
	}
}
