#include <gpt/structures.h>
#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "gpt_partition"



static _Bool _gpt_load_partitions(drive_t* drive){
	if (drive->block_size>=4096){
		return 0;
	}
	u8 buffer[4096];
	if (drive_read(drive,1,buffer,1)!=1){
		return 0;
	}
	gpt_table_header_t* header=(void*)buffer;
	if (header->signature!=GPT_TABLE_HEADER_SIGNATURE){
		return 0;
	}
	return 0;
	// panic("_gpt_load_partitions");
}



static partition_table_descriptor_t _gpt_partition_table_descriptor={
	"GPT",
	_gpt_load_partitions
};



void gpt_register_partition_table(void){
	partition_register_table_descriptor(&_gpt_partition_table_descriptor);
}
