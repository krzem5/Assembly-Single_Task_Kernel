#include <iso9660/structures.h>
#include <kernel/drive/drive.h>
#include <kernel/module/module.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#define KERNEL_LOG_NAME "iso9660_partition"



static _Bool _iso9660_load_partitions(drive_t* drive){
	if (drive->block_size!=2048||(!str_equal(drive->type->name,"ATA")&&!str_equal(drive->type->name,"ATAPI"))){
		return 0;
	}
	u64 block_index=16;
	u8 buffer[2048];
	while (1){
		if (drive_read(drive,block_index,buffer,1)!=1){
			return 0;
		}
		iso9660_volume_descriptor_t* volume_descriptor=(iso9660_volume_descriptor_t*)buffer;
		if (volume_descriptor->identifier[0]!='C'||volume_descriptor->identifier[1]!='D'||volume_descriptor->identifier[2]!='0'||volume_descriptor->identifier[3]!='0'||volume_descriptor->identifier[4]!='1'||volume_descriptor->version!=1){
			return 0;
		}
		switch (volume_descriptor->type){
			case 1:
				char name_buffer[32];
				str_copy_from_padded(volume_descriptor->primary_volume_descriptor.volume_name,name_buffer,32);
				partition_create(drive,block_index-16,name_buffer,0,volume_descriptor->primary_volume_descriptor.volume_size);
				return 1;
			case 255:
				return 0;
		}
		block_index++;
	}
}



static const partition_table_descriptor_config_t _iso9660_partition_table_descriptor={
	"iso9660",
	_iso9660_load_partitions,
	NULL
};



MODULE_POSTINIT(){
	partition_register_table_descriptor(&_iso9660_partition_table_descriptor);
}
