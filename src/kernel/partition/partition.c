#include <kernel/drive/drive.h>
#include <kernel/vfs/vfs.h>
#include <kernel/vfs/allocator.h>
#include <kernel/partition/partition.h>
#include <kernel/fs/emptyfs.h>
#include <kernel/fs/iso9660.h>
#include <kernel/fs/kfs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
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



typedef struct __attribute__((packed)) _KFS_ROOT_BLOCK{
	u64 signature;
} kfs_root_block_t;



partition_t KERNEL_CORE_BSS partition_data[MAX_PARTITIONS];
u8 KERNEL_CORE_BSS partition_count;
u8 KERNEL_CORE_DATA partition_boot_index=PARTITION_INVALID_INDEX;



static void KERNEL_CORE_CODE _load_iso9660(drive_t* drive){
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
			INFO_CORE("Detected drive format of '%s' as ISO 9660",drive->model_number);
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
			WARN_CORE("Too many ISO 9660 partitions");
			break;
		}
		partition_index++;
	}
_loaded_all_blocks:
	return;
}



static void KERNEL_CORE_CODE _load_kfs(const drive_t* drive){
	if (drive->block_size>4096){
		return;
	}
	u8 buffer[4096];
	if (drive->read_write(drive->extra_data,1,buffer,1)!=1){
		return;
	}
	const kfs_root_block_t* kfs_root_block=(const kfs_root_block_t*)buffer;
	if (kfs_root_block->signature!=KFS_SIGNATURE){
		return;
	}
	INFO_CORE("Detected drive format of '%s' as KFS",drive->model_number);
	const partition_config_t partition_config={
		PARTITION_CONFIG_TYPE_KFS,
		0,
		0,
		drive->block_count
	};
	kfs_load(drive,&partition_config);
}



void* KERNEL_CORE_CODE partition_add(const drive_t* drive,const partition_config_t* partition_config,const partition_file_system_config_t* config,void* extra_data){
	if (partition_count>=MAX_PARTITIONS){
		ERROR_CORE("Too many partitions!");
		return NULL;
	}
	partition_t* fs=partition_data+partition_count;
	partition_count++;
	lock_init(&(fs->lock));
	fs->config=config;
	fs->partition_config=*partition_config;
	fs->index=partition_count-1;
	fs->flags=0;
	u8 i=0;
	while (drive->name[i]){
		fs->name[i]=drive->name[i];
		i++;
	}
	if (partition_config->type==PARTITION_CONFIG_TYPE_DRIVE){
		fs->name[i]=0;
		fs->name_length=i;
	}
	else if (partition_config->index<10){
		fs->name[i]='p';
		fs->name[i+1]=partition_config->index+48;
		fs->name[i+2]=0;
		fs->name_length=i+2;
	}
	else if (partition_config->index<100){
		fs->name[i]='p';
		fs->name[i+1]=partition_config->index/10+48;
		fs->name[i+2]=(partition_config->index%10)+48;
		fs->name[i+3]=0;
		fs->name_length=i+3;
	}
	else{
		fs->name[i]='p';
		fs->name[i+1]=partition_config->index/100+48;
		fs->name[i+2]=((partition_config->index/10)%10)+48;
		fs->name[i+3]=(partition_config->index%10)+48;
		fs->name[i+4]=0;
		fs->name_length=i+4;
	}
	fs->drive=drive;
	fs->extra_data=extra_data;
	vfs_allocator_init(partition_count-1,config->node_size,&(fs->allocator));
	LOG_CORE("Created partition '%s' from drive '%s'",fs->name,drive->model_number);
	fs->root=vfs_alloc(fs->index,"",0);
	fs->root->type=VFS_NODE_TYPE_DIRECTORY;
	fs->root->flags|=VFS_NODE_FLAG_ROOT;
	fs->root->parent=fs->root->id;
	fs->root->prev_sibling=fs->root->id;
	fs->root->next_sibling=fs->root->id;
	return fs->root;
}



void KERNEL_CORE_CODE partition_load_from_drive(drive_t* drive){
	LOG_CORE("Loading partitions from drive '%s'...",drive->model_number);
	_load_iso9660(drive);
	_load_kfs(drive);
	const partition_config_t partition_config={
		PARTITION_CONFIG_TYPE_DRIVE,
		0,
		0,
		drive->block_count
	};
	emptyfs_load(drive,&partition_config);
}



void partition_flush_cache(void){
	LOG("Flushing partition cache...");
	for (u8 i=0;i<partition_count;i++){
		partition_t* fs=partition_data+i;
		lock_acquire_exclusive(&(fs->lock));
		fs->config->flush_cache(fs);
		lock_release_exclusive(&(fs->lock));
	}
}
