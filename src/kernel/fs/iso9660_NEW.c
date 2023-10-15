#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "iso9660"



typedef struct __attribute__((packed)) _ISO9660_VOLUME_DESCRIPTOR{
	u8 type;
	u8 identifier[5];
	u8 version;
	struct __attribute__((packed)){
		u8 _padding[151];
		u32 directory_lba;
		u8 _padding2[4];
		u32 directory_data_length;
	} primary_volume_descriptor;
} iso9660_volume_descriptor_t;



extern filesystem_type_t FILESYSTEM_TYPE_ISO9660;



static void _iso9660_deinit_callback(filesystem2_t* fs){
	panic("_iso9660_deinit_callback");
}



static filesystem2_t* _iso9660_load_callback(partition2_t* partition){
	if (partition->start_lba||partition->drive->type!=DRIVE_TYPE_ATAPI||partition->drive->block_size!=2048){
		return NULL;
	}
	u32 directory_lba=0;
	u32 directory_data_length=0;
	u64 block_index=16;
	u8 buffer[2048];
	while (1){
		if (partition->drive->read_write(partition->drive->extra_data,block_index,buffer,1)!=1){
			return NULL;
		}
		iso9660_volume_descriptor_t* volume_descriptor=(iso9660_volume_descriptor_t*)buffer;
		if (volume_descriptor->identifier[0]!='C'||volume_descriptor->identifier[1]!='D'||volume_descriptor->identifier[2]!='0'||volume_descriptor->identifier[3]!='0'||volume_descriptor->identifier[4]!='1'||volume_descriptor->version!=1){
			return NULL;
		}
		switch (volume_descriptor->type){
			case 1:
				directory_lba=volume_descriptor->primary_volume_descriptor.directory_lba;
				directory_data_length=volume_descriptor->primary_volume_descriptor.directory_data_length;
				goto _directory_lba_found;
			case 255:
				return NULL;
		}
		block_index++;
	}
_directory_lba_found:
	filesystem2_t* out=fs_create(FILESYSTEM_TYPE_ISO9660);
	WARN("%u %u",directory_lba,directory_data_length);
	return out;
}



FILESYSTEM_DECLARE_TYPE(
	ISO9660,
	_iso9660_deinit_callback,
	_iso9660_load_callback
)
