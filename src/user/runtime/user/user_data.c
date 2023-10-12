#include <user/aml.h>
#include <user/cpu.h>
#include <user/drive.h>
#include <user/memory.h>
#include <user/network.h>
#include <user/numa.h>
#include <user/partition.h>
#include <user/syscall.h>
#include <user/system.h>
#include <user/types.h>



typedef struct _USER_DATA_HEADER{
	const system_bios_data_t* bios_data;
	u32 drive_count;
	u32 drive_boot_index;
	const drive_t* drives;
	u32 partition_count;
	u32 partition_boot_index;
	const partition_t* partitions;
	const network_config_t* layer1_network_device;
	u32 memory_range_count;
	const memory_range_t* memory_ranges;
	u32 cpu_count;
	const cpu_t* cpus;
} user_data_header_t;



const system_bios_data_t* system_bios_data;
u32 drive_count;
u32 drive_boot_index;
const drive_t* drives;
u32 partition_count;
u32 partition_boot_index;
const partition_t* partitions;
const network_config_t* network_config;
u32 memory_range_count;
const memory_range_t* memory_ranges;
u32 cpu_count;
const cpu_t* cpus;



void _user_data_init(void){
	const user_data_header_t* header=_syscall_user_data_pointer();
	system_bios_data=header->bios_data;
	drive_count=header->drive_count;
	drive_boot_index=header->drive_boot_index;
	drives=header->drives;
	partition_count=header->partition_count;
	partition_boot_index=header->partition_boot_index;
	partitions=header->partitions;
	network_config=header->layer1_network_device;
	memory_range_count=header->memory_range_count;
	memory_ranges=header->memory_ranges;
	cpu_count=header->cpu_count;
	cpus=header->cpus;
}
