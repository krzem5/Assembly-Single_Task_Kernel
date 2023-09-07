#include <kernel/aml/runtime.h>
#include <kernel/bios/bios.h>
#include <kernel/config.h>
#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
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



typedef struct _USER_NUMA_CPU{
	u8 apic_id;
	u32 sapic_eid;
} user_numa_cpu_t;



typedef struct _USER_NUMA_MEMORY_RANGE{
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



typedef struct _USER_MEMORY_RANGE{
	u64 base_address;
	u64 length;
	u32 type;
} user_memory_range_t;



typedef struct _USER_DATA_HEADER{
	user_bios_data_t* bios_data;
	u32 drive_count;
	u32 drive_boot_index;
	user_drive_t* drives;
	u32 partition_count;
	u32 partition_boot_index;
	user_partition_t* partitions;
	u32 numa_node_count;
	user_numa_node_t* numa_nodes;
	u8* numa_node_locality_matrix;
	user_layer1_network_device_t* layer1_network_device;
	u32 memory_range_count;
	user_memory_range_t* memory_ranges;
	void* aml_root_node;
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



static void _generate_bios_data(user_data_header_t* header){
	user_bios_data_t* user_bios_data=umm_alloc(sizeof(user_bios_data_t));
	if (CONFIG_DISABLE_USER_BIOS_DATA){
		char* unknown_string=_duplicate_string("???");
		user_bios_data->bios_vendor=unknown_string;
		user_bios_data->bios_version=unknown_string;
		user_bios_data->manufacturer=unknown_string;
		user_bios_data->product=unknown_string;
		user_bios_data->version=unknown_string;
		user_bios_data->serial_number=unknown_string;
		memset(user_bios_data->uuid,0,16);
		user_bios_data->wakeup_type=BIOS_DATA_WAKEUP_TYPE_UNKNOWN;
	}
	else{
		user_bios_data->bios_vendor=_duplicate_string(bios_data.bios_vendor);
		user_bios_data->bios_version=_duplicate_string(bios_data.bios_version);
		user_bios_data->manufacturer=_duplicate_string(bios_data.manufacturer);
		user_bios_data->product=_duplicate_string(bios_data.product);
		user_bios_data->version=_duplicate_string(bios_data.version);
		user_bios_data->serial_number=_duplicate_string(bios_data.serial_number);
		memcpy(user_bios_data->uuid,bios_data.uuid,16);
		user_bios_data->wakeup_type=bios_data.wakeup_type;
	}
	header->bios_data=user_bios_data;
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



static void _generate_numa_nodes(user_data_header_t* header){
	if (CONFIG_DISABLE_USER_NUMA){
		header->numa_node_count=0;
		header->numa_nodes=NULL;
		header->numa_node_locality_matrix=NULL;
		return;
	}
	header->numa_node_count=numa_node_count;
	header->numa_nodes=umm_alloc(numa_node_count*sizeof(user_numa_node_t));
	user_numa_node_t* user_numa_node=header->numa_nodes;
	for (u32 i=0;i<numa_node_count;i++){
		const numa_node_t* numa_node=numa_nodes+i;
		user_numa_node->index=numa_node->index;
		user_numa_node->cpu_count=numa_node->cpu_count;
		user_numa_node->memory_range_count=numa_node->memory_range_count;
		user_numa_node->cpus=umm_alloc(numa_node->cpu_count*sizeof(user_numa_cpu_t));
		user_numa_cpu_t* user_numa_cpu=user_numa_node->cpus;
		for (const numa_cpu_t* cpu=numa_node->cpus;cpu;cpu=cpu->next){
			user_numa_cpu->apic_id=cpu->apic_id;
			user_numa_cpu->sapic_eid=cpu->sapic_eid;
			user_numa_cpu++;
		}
		user_numa_node->memory_ranges=umm_alloc(numa_node->memory_range_count*sizeof(user_numa_memory_range_t));
		user_numa_memory_range_t* user_numa_memory_range=user_numa_node->memory_ranges;
		for (const numa_memory_range_t* memory_range=numa_node->memory_ranges;memory_range;memory_range=memory_range->next){
			user_numa_memory_range->base_address=memory_range->base_address;
			user_numa_memory_range->length=memory_range->length;
			user_numa_memory_range->hot_pluggable=memory_range->hot_pluggable;
			user_numa_memory_range++;
		}
		user_numa_node++;
	}
	header->numa_node_locality_matrix=umm_alloc(numa_node_count*numa_node_count);
	memcpy(header->numa_node_locality_matrix,numa_node_locality_matrix,numa_node_count*numa_node_count);
}



static void _generate_layer1_network_device(user_data_header_t* header){
	if (!network_layer1_name){
		header->layer1_network_device=NULL;
		return;
	}
	user_layer1_network_device_t* user_layer1_network_device=umm_alloc(sizeof(user_layer1_network_device_t));
	user_layer1_network_device->name=_duplicate_string(network_layer1_name);
	if (CONFIG_DISABLE_USER_MAC_ADDRESS){
		memset(user_layer1_network_device->mac_address,0,6);
	}
	else{
		memcpy(user_layer1_network_device->mac_address,network_layer1_mac_address,6);
	}
	header->layer1_network_device=user_layer1_network_device;
}



static void _generate_memory_ranges(user_data_header_t* header){
	if (CONFIG_DISABLE_USER_MEMORY_RANGES){
		header->memory_range_count=0;
		header->memory_ranges=NULL;
		return;
	}
	header->memory_range_count=KERNEL_DATA->mmap_size;
	header->memory_ranges=umm_alloc(KERNEL_DATA->mmap_size*sizeof(user_numa_memory_range_t));
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		(header->memory_ranges+i)->base_address=(KERNEL_DATA->mmap+i)->base;
		(header->memory_ranges+i)->length=(KERNEL_DATA->mmap+i)->length;
		(header->memory_ranges+i)->type=(KERNEL_DATA->mmap+i)->type;
	}
}



static void _generate_aml_root_node(user_data_header_t* header){
	header->aml_root_node=(CONFIG_DISABLE_USER_AML?NULL:aml_root_node);
}



void user_data_generate(void){
	LOG("Generating user data structures...");
	user_data_header_t* header=umm_alloc(sizeof(user_data_header_t));
	_generate_bios_data(header);
	_generate_drives(header);
	_generate_partitions(header);
	_generate_numa_nodes(header);
	_generate_layer1_network_device(header);
	_generate_memory_ranges(header);
	_generate_aml_root_node(header);
	user_data_pointer=header;
}
