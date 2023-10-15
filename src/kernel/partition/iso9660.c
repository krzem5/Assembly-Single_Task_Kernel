#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



typedef struct __attribute__((packed)) _ISO9660_VOLUME_DESCRIPTOR{
	u8 type;
	u8 identifier[5];
	u8 version;
	struct __attribute__((packed)){
		u8 _padding[33];
		char volume_name[32];
		u8 _padding2[8];
		u32 volume_size;
		u8 _padding3[74];
		u32 directory_lba;
		u8 _padding4[4];
		u32 directory_data_length;
	} primary_volume_descriptor;
} iso9660_volume_descriptor_t;



PARTITION_DECLARE_TYPE(ISO9660,{
	if (drive->type!=DRIVE_TYPE_ATAPI||drive->block_size!=2048){
		return 0;
	}
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
		switch (volume_descriptor->type){
			case 1:
				char name_buffer[32];
				memcpy_trunc_spaces(name_buffer,volume_descriptor->primary_volume_descriptor.volume_name,32);
				partition_create(drive,name_buffer,0,volume_descriptor->primary_volume_descriptor.volume_size);
				return 1;
			case 255:
				return 0;
		}
		block_index++;
	}
});
