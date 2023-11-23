#include <gpt/structures.h>
#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "gpt_partition"



static pmm_counter_descriptor_t* _gpt_driver_pmm_counter;



static _Bool _gpt_load_partitions(drive_t* drive){
	if (drive->block_size>=4096){
		return 0;
	}
	u8 buffer[4096];
	if (drive_read(drive,1,buffer,1)!=1){
		return 0;
	}
	const gpt_table_header_t* header=(void*)buffer;
	if (header->signature!=GPT_TABLE_HEADER_SIGNATURE){
		return 0;
	}
	u32 entry_buffer_size=header->entry_count*header->entry_size;
	void* entry_buffer=(void*)(pmm_alloc(pmm_align_up_address(entry_buffer_size)>>PAGE_SIZE_SHIFT,_gpt_driver_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u32 aligned_entry_buffer_size=(entry_buffer_size+drive->block_size-1)>>drive->block_size_shift;
	if (drive_read(drive,header->entry_lba,entry_buffer,aligned_entry_buffer_size)!=aligned_entry_buffer_size){
		pmm_dealloc(((u64)entry_buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(entry_buffer_size)>>PAGE_SIZE_SHIFT,_gpt_driver_pmm_counter);
		return 0;
	}
	INFO("Found valid GPT partition table: %g",header->guid);
	for (u32 i=0;i<entry_buffer_size;i+=header->entry_size){
		const gpt_partition_entry_t* entry=entry_buffer+i;
		for (u8 j=0;j<16;j++){
			if (entry->type_guid[j]){
				goto _valid_entry;
			}
		}
		continue;
_valid_entry:
		char name_buffer[32];
		u8 j=0;
		for (;j<((header->entry_size-56)>>1)&&j<31;j++){
			name_buffer[j]=entry->name[j];
		}
		name_buffer[j]=0;
		partition_create(drive,i/header->entry_size,name_buffer,entry->start_lba,entry->end_lba);
	}
	pmm_dealloc(((u64)entry_buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(entry_buffer_size)>>PAGE_SIZE_SHIFT,_gpt_driver_pmm_counter);
	return 1;
}



static const partition_table_descriptor_config_t _gpt_partition_table_descriptor={
	"gpt",
	_gpt_load_partitions
};



void gpt_register_partition_table(void){
	_gpt_driver_pmm_counter=pmm_alloc_counter("gpt");
	partition_register_table_descriptor(&_gpt_partition_table_descriptor);
}
