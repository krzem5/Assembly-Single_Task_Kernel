#include <kernel/drive/drive.h>
#include <kernel/fs/emptyfs.h>
#include <kernel/fs/iso9660.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



typedef struct __attribute__((packed)) _GPT_PARTITION_TABLE_HEADER{
	u64 signature;
	u32 revision;
	u32 header_size;
	u8 _padding[56];
	u64 partition_entry_array_lba;
	u32 partition_entry_count;
	u32 partition_entry_size;
} gpt_partition_table_header_t;



typedef struct __attribute__((packed)) _GPT_PARTITION_ENTRY{
	u64 type_guid[2];
	u8 _padding[16];
	u64 start_lba;
	u64 end_lba;
} gpt_partition_entry_t;



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



static void _try_load_file_system(drive_t* drive,const fs_partition_config_t* partition_config,u64 first_block_index,u64 last_block_index){
	LOG("Probing drive '%s' block %u for possible file systems...",drive->model_number,first_block_index);
	u8 buffer[4096];
	if (drive->read_write(drive->extra_data,first_block_index,buffer,1)!=1){
		return;
	}
	// Try to load different file systems and fall back to emptyfs
	INFO("Probing failed, loading partition as emptyfs...");
	fs_emptyfs_load(drive,partition_config);
}



static _Bool _load_gpt(drive_t* drive){
	if (drive->block_size>4096){
		return 0;
	}
	u8 buffer[4096];
	if (drive->read_write(drive->extra_data,1,buffer,1)!=1){
		return 0;
	}
	const gpt_partition_table_header_t* gpt_header=(const gpt_partition_table_header_t*)buffer;
	if (gpt_header->signature!=0x5452415020494645ull){
		return 0;
	}
	INFO("Detected drive format of '%s' as GPT",drive->model_number);
	if (gpt_header->partition_entry_size>drive->block_size){
		WARN("Unimplemented");
		return 0;
	}
	u16 buffer_space=0;
	u64 block_index=gpt_header->partition_entry_array_lba;
	u32 partition_count=gpt_header->partition_entry_count;
	if (partition_count>256){
		WARN("Too many GPT partitions");
		partition_count=256;
	}
	u32 partition_entry_size=gpt_header->partition_entry_size;
	for (u32 i=0;i<partition_count;i++){
		if (!buffer_space){
			buffer_space=drive->block_size;
			if (drive->read_write(drive->extra_data,block_index,buffer,1)!=1){
				return 0;
			}
			block_index++;
		}
		const gpt_partition_entry_t* gpt_partition_entry=(const gpt_partition_entry_t*)(buffer+drive->block_size-buffer_space);
		if (gpt_partition_entry->type_guid[0]||gpt_partition_entry->type_guid[1]){
			const fs_partition_config_t partition_config={
				FS_PARTITION_TYPE_GPT,
				i,
				gpt_partition_entry->start_lba,
				gpt_partition_entry->end_lba
			};
			_try_load_file_system(drive,&partition_config,gpt_partition_entry->start_lba,gpt_partition_entry->end_lba);
		}
		buffer_space-=partition_entry_size;
	}
	return 1;
}



static _Bool _load_iso9660(drive_t* drive){
	if (drive->type!=DRIVE_TYPE_ATAPI||drive->block_size!=2048){
		return 0;
	}
	u8 partition_index=0;
	u64 block_index=16;
	u8 buffer[2048];
	while (1){
		if (drive->read_write(drive->extra_data,block_index,buffer,1)!=1){
			return 0;
		}
		iso9660_volume_descriptor_t* volume_descriptor=(iso9660_volume_descriptor_t*)buffer;
		if (volume_descriptor->identifier[0]!='C'||volume_descriptor->identifier[1]!='D'||volume_descriptor->identifier[2]!='0'||volume_descriptor->identifier[3]!='0'||volume_descriptor->identifier[4]!='1'||volume_descriptor->version!=1){
			return 0;
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
				const fs_partition_config_t partition_config={
					FS_PARTITION_TYPE_ISO9660,
					partition_index,
					0,
					volume_descriptor->primary_volume_descriptor.volume_size
				};
				fs_iso9660_load(drive,&partition_config,volume_descriptor->primary_volume_descriptor.directory_lba,volume_descriptor->primary_volume_descriptor.directory_data_length);
				break;
			case 255:
				goto _loaded_all_blocks;
			default:
				return 0;
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
	return 1;
}



void fs_partition_load_from_drive(drive_t* drive){
	LOG("Loading partitions from drive '%s'...",drive->model_number);
	u8 loaded=_load_gpt(drive);
	loaded|=_load_iso9660(drive);
	if (loaded){
		return;
	}
	LOG("Failed to detected drive format, partitioning as emptyfs...");
	const fs_partition_config_t partition_config={
		FS_PARTITION_TYPE_EMPTY_DRIVE,
		0,
		0,
		drive->block_count
	};
	fs_emptyfs_load(drive,&partition_config);
	for (;;);
}
