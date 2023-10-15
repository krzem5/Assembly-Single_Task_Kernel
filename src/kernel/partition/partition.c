#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/fs/emptyfs.h>
#include <kernel/fs/iso9660.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/allocator.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "partition"



typedef struct __attribute__((packed)) _ISO9660_VOLUME_DESCRIPTOR{
	u8 type;
	u8 identifier[5];
	u8 version;
	struct __attribute__((packed)){
		u8 _padding[73];
		u32 volume_size;
		u8 _padding2[74];
		u32 directory_lba;
		u8 _padding3[4];
		u32 directory_data_length;
	} primary_volume_descriptor;
} iso9660_volume_descriptor_t;



static u8 KERNEL_BSS _partition_count;
static partition_t** KERNEL_BSS _partition_lookup_table;

partition_t* KERNEL_BSS partition_data;
partition_t* KERNEL_BSS partition_boot;



static void _load_iso9660(drive_t* drive){
	if (drive->type!=DRIVE_TYPE_ATAPI||drive->block_size!=2048){
		return;
	}
	u8 partition_index=0;
	u64 block_index=16;
	u8 buffer[2048];
	while (1){
		if (drive->read_write(drive->extra_data,block_index,buffer,1)!=1){
			return;
		}
		iso9660_volume_descriptor_t* volume_descriptor=(iso9660_volume_descriptor_t*)buffer;
		if (volume_descriptor->identifier[0]!='C'||volume_descriptor->identifier[1]!='D'||volume_descriptor->identifier[2]!='0'||volume_descriptor->identifier[3]!='0'||volume_descriptor->identifier[4]!='1'||volume_descriptor->version!=1){
			return;
		}
		if (block_index==16){
			INFO("Detected drive format of '%s' as ISO 9660",drive->model_number);
		}
		switch (volume_descriptor->type){
			case 0:
			case 2:
			case 3:
				goto _load_next_block;
			case 1:
				const partition_config_t partition_config={
					PARTITION_CONFIG_TYPE_ISO9660,
					partition_index,
					0,
					volume_descriptor->primary_volume_descriptor.volume_size
				};
				iso9660_load(drive,&partition_config,volume_descriptor->primary_volume_descriptor.directory_lba,volume_descriptor->primary_volume_descriptor.directory_data_length);
				break;
			case 255:
				goto _loaded_all_blocks;
			default:
				return;
		}
_load_next_block:
		block_index++;
		if (partition_index==255){
			WARN("Too many ISO 9660 partitions");
			break;
		}
		partition_index++;
	}
_loaded_all_blocks:
	return;
}



void* partition_add(const drive_t* drive,const partition_config_t* partition_config,const partition_file_system_config_t* config,void* extra_data){
	if (_partition_lookup_table){
		panic("Unable to add partition");
	}
	partition_t* fs=kmm_alloc(sizeof(partition_t));
	fs->next=partition_data;
	partition_data=fs;
	lock_init(&(fs->lock));
	fs->config=config;
	fs->partition_config=*partition_config;
	fs->index=_partition_count;
	fs->flags=0;
	fs->name_length=format_string(fs->name,16,(partition_config->type==PARTITION_CONFIG_TYPE_DRIVE?"%s":"%sp%u"),drive->name,partition_config->index);
	fs->drive=drive;
	fs->extra_data=extra_data;
	vfs_allocator_init(_partition_count,config->node_size,&(fs->allocator));
	LOG("Created partition '%s' from drive '%s'",fs->name,drive->model_number);
	fs->root=vfs_alloc(fs,"",0);
	fs->root->type=VFS_NODE_TYPE_DIRECTORY;
	fs->root->flags|=VFS_NODE_FLAG_ROOT;
	fs->root->parent=fs->root->id;
	fs->root->prev_sibling=fs->root->id;
	fs->root->next_sibling=fs->root->id;
	_partition_count++;
	return fs->root;
}



partition_t* partition_get(u32 index){
	if (!_partition_lookup_table){
		panic("Unable to get partition");
	}
	return (index>=_partition_count?NULL:_partition_lookup_table[index]);
}



void partition_load(void){
	LOG("Loading drive partitions...");
	for (drive_t* drive=drive_data;drive;drive=drive->next){
		INFO("Loading partitions from drive '%s'...",drive->model_number);
		_load_iso9660(drive);
		const partition_config_t partition_config={
			PARTITION_CONFIG_TYPE_DRIVE,
			0,
			0,
			drive->block_count
		};
		emptyfs_load(drive,&partition_config);
	}
	LOG("Building partition lookup table...");
	_partition_lookup_table=kmm_alloc(_partition_count*sizeof(partition_t*));
	for (partition_t* partition=partition_data;partition;partition=partition->next){
		_partition_lookup_table[partition->index]=partition;
	}
}



void partition_flush_cache(void){
	LOG("Flushing partition cache...");
	for (partition_t* fs=partition_data;fs;fs=fs->next){
		lock_acquire_exclusive(&(fs->lock));
		fs->config->flush_cache(fs);
		lock_release_exclusive(&(fs->lock));
	}
}
