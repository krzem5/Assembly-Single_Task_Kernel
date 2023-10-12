#include <kernel/aml/runtime.h>
#include <kernel/bios/bios.h>
#include <kernel/cpu/cpu.h>
#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/umm.h>
#include <kernel/network/layer1.h>
#include <kernel/numa/numa.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "user_data"



typedef struct _USER_DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char* name;
	char* serial_number;
	char* model_number;
	u64 block_count;
	u64 block_size;
} user_drive_t;



typedef struct _USER_PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char* name;
	u32 drive_index;
} user_partition_t;



typedef struct _USER_MEMORY_RANGE{
	u64 base_address;
	u64 length;
	u32 type;
} user_memory_range_t;



typedef struct _USER_CPU{
	u32 domain;
	u32 chip;
	u32 core;
	u32 thread;
} user_cpu_t;



typedef struct _USER_DATA_HEADER{
	u32 drive_count;
	u32 drive_boot_index;
	user_drive_t* drives;
	u32 partition_count;
	u32 partition_boot_index;
	user_partition_t* partitions;
	u32 memory_range_count;
	user_memory_range_t* memory_ranges;
	u32 cpu_count;
	user_cpu_t* cpus;
} user_data_header_t;



void* user_data_pointer;



static char* _duplicate_string(const char* str){
	u32 length=0;
	do{
		length++;
	} while (str[length-1]);
	char* out=umm_alloc(length);
	memcpy(out,str,length);
	return out;
}



static void _generate_drives(user_data_header_t* header){
	header->drive_count=0;
	for (const drive_t* drive=drive_data;drive;drive=drive->next){
		header->drive_count++;
	}
	header->drives=umm_alloc(header->drive_count*sizeof(user_drive_t));
	for (const drive_t* drive=drive_data;drive;drive=drive->next){
		user_drive_t* user_drive=header->drives+drive->index;
		user_drive->flags=drive->flags;
		user_drive->type=drive->type;
		user_drive->index=drive->index;
		user_drive->name=_duplicate_string(drive->name);
		user_drive->serial_number=_duplicate_string(drive->serial_number);
		user_drive->model_number=_duplicate_string(drive->model_number);
		user_drive->block_count=drive->block_count;
		user_drive->block_size=drive->block_size;
		if (drive->flags&DRIVE_FLAG_BOOT){
			header->drive_boot_index=drive->index;
		}
	}
}



static void _generate_partitions(user_data_header_t* header){
	header->partition_count=0;
	for (const partition_t* partition=partition_data;partition;partition=partition->next){
		header->partition_count++;
	}
	header->partition_boot_index=partition_boot->index;
	header->partitions=umm_alloc(header->partition_count*sizeof(user_partition_t));
	for (const partition_t* partition=partition_data;partition;partition=partition->next){
		user_partition_t* user_partition=header->partitions+partition->index;
		user_partition->flags=partition->flags;
		user_partition->type=partition->partition_config.type;
		user_partition->index=partition->partition_config.index;
		user_partition->first_block_index=partition->partition_config.first_block_index;
		user_partition->last_block_index=partition->partition_config.last_block_index;
		user_partition->name=_duplicate_string(partition->name);
		user_partition->drive_index=partition->drive->index;
	}
}



static void _generate_memory_ranges(user_data_header_t* header){
	header->memory_range_count=kernel_data.mmap_size;
	header->memory_ranges=umm_alloc(kernel_data.mmap_size*sizeof(user_memory_range_t));
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		(header->memory_ranges+i)->base_address=(kernel_data.mmap+i)->base;
		(header->memory_ranges+i)->length=(kernel_data.mmap+i)->length;
		(header->memory_ranges+i)->type=(kernel_data.mmap+i)->type;
	}
}



static void _generate_user_cpus(user_data_header_t* header){
	header->cpu_count=cpu_count;
	header->cpus=umm_alloc(cpu_count*sizeof(user_cpu_t));
	for (u16 i=0;i<cpu_count;i++){
		(header->cpus+i)->domain=(cpu_extra_data+i)->topology.domain;
		(header->cpus+i)->chip=(cpu_extra_data+i)->topology.chip;
		(header->cpus+i)->core=(cpu_extra_data+i)->topology.core;
		(header->cpus+i)->thread=(cpu_extra_data+i)->topology.thread;
	}
}



void user_data_generate(void){
	LOG("Generating user data structures...");
	user_data_header_t* header=umm_alloc(sizeof(user_data_header_t));
	_generate_drives(header);
	_generate_partitions(header);
	_generate_memory_ranges(header);
	_generate_user_cpus(header);
	user_data_pointer=header;
}
