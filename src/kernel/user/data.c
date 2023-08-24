#include <kernel/bios/bios.h>
#include <kernel/drive/drive_list.h>
#include <kernel/log/log.h>
#include <kernel/memory/umm.h>
#include <kernel/network/layer1.h>
#include <kernel/numa/numa.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "user_data"



typedef struct _USER_BIOS_DATA{
	char* bios_vendor;
	char* bios_version;
	char* manufacturer;
	char* product;
	char* version;
	char* serial_number;
	u8 uuid[16];
	u8 wakeup_type;
} user_bios_data_t;



typedef struct _USER_DRIVE{
	struct _USER_DRIVE* next;
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
	struct _USER_PARTITION* next;
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char* name;
	u32 drive_index;
} user_partition_t;



typedef struct _USER_NUMA_CPU{
	struct _USER_NUMA_CPU* next;
	u8 apic_id;
	u32 sapic_eid;
} user_numa_cpu_t;



typedef struct _USER_NUMA_MEMORY_RANGE{
	struct _USER_NUMA_MEMORY_RANGE* next;
	u64 base_address;
	u64 length;
	_Bool hot_pluggable;
} user_numa_memory_range_t;



typedef struct _USER_NUMA_NODE{
	u32 index;
	u32 cpu_count;
	u32 memory_range_count;
	user_numa_cpu_t* cpus;
	user_numa_memory_range_t* memory_ranges;
} user_numa_node_t;



typedef struct _USER_LAYER1_NETWORK_DEVICE{
	char* name;
	u8 mac_address[6];
} user_layer1_network_device_t;



typedef struct _USER_DATA_HEADER{
	user_bios_data_t* bios_data;
	u32 drive_count;
	user_drive_t* drives;
	u32 partition_count;
	user_partition_t* partitions;
	u32 numa_node_count;
	user_numa_node_t* numa_nodes;
	u8* numa_node_locality_matrix;
	user_layer1_network_device_t* layer1_network_device;
} user_data_header_t;



static char* _duplicate_string(const char* str){
	u32 length=0;
	do{
		length++;
	} while (str[length-1]);
	char* out=umm_alloc(length);
	memcpy(out,str,length);
	return out;
}



static void _generate_bios_data(user_data_header_t* header){
	header->bios_data=NULL;
}



static void _generate_drives(user_data_header_t* header){
	header->drive_count=0;
	header->drives=NULL;
}



static void _generate_partitions(user_data_header_t* header){
	header->partition_count=0;
	header->partitions=NULL;
}



static void _generate_numa_nodes(user_data_header_t* header){
	header->numa_node_count=0;
	header->numa_nodes=NULL;
	header->numa_node_locality_matrix=NULL;
}



static void _generate_layer1_network_device(user_data_header_t* header){
	if (!network_layer1_name){
		header->layer1_network_device=NULL;
		return;
	}
	user_layer1_network_device_t* layer1_network_device=umm_alloc(sizeof(user_layer1_network_device_t));
	layer1_network_device->name=_duplicate_string(network_layer1_name);
	memcpy(layer1_network_device->mac_address,network_layer1_mac_address,6);
	header->layer1_network_device=layer1_network_device;
}



void user_data_generate(void){
	LOG("Generating user data structures...");
	user_data_header_t* header=umm_alloc(sizeof(user_data_header_t));
	_generate_bios_data(header);
	_generate_drives(header);
	_generate_partitions(header);
	_generate_numa_nodes(header);
	_generate_layer1_network_device(header);
}
