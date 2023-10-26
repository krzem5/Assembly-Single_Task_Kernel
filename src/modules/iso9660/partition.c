#include <iso9660/structures.h>
#include <kernel/drive/drive.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "iso9660_partition"



static _Bool _iso9660_load_partitions(drive_t* drive){
	if (drive->block_size!=2048||(!streq(drive->type->name,"ATA")&&!streq(drive->type->name,"ATAPI"))){
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
				memcpy_trunc_spaces(name_buffer,volume_descriptor->primary_volume_descriptor.volume_name,32);
				partition_create(drive,name_buffer,0,volume_descriptor->primary_volume_descriptor.volume_size);
				return 1;
			case 255:
				return 0;
		}
		block_index++;
	}
}



static partition_table_descriptor_t _iso9660_partition_table_descriptor={
	"ISO9660",
	_iso9660_load_partitions
};



void iso9660_register_partition_table(void){
	partition_register_table_descriptor(&_iso9660_partition_table_descriptor);
}
