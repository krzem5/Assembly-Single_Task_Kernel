#ifndef _KERNEL_PARTITION_PARTITION_H_
#define _KERNEL_PARTITION_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/memory/smm.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>



#define PARTITION_TABLE_DESCRIPTOR_FLAG_CAN_FORMAT 1



typedef struct _PARTITION{
	handle_t handle;
	partition_table_descriptor_t* descriptor;
	drive_t* drive;
	u32 index;
	string_t* name;
	u64 start_lba;
	u64 end_lba;
	u8 guid[16];
	filesystem_t* fs;
} partition_t;



typedef struct _PARTITION_USER_DATA{
	char name[64];
	char type[64];
	u64 drive;
	u32 index;
	u64 start_lba;
	u64 end_lba;
	u64 fs;
} partition_user_data_t;



typedef struct _PARTITION_TABLE_DESCRIPTOR_USER_DATA{
	char name[64];
	u32 flags;
} partition_table_descriptor_user_data_t;



extern handle_type_t partition_handle_type;
extern handle_type_t partition_table_descriptor_handle_type;



partition_table_descriptor_t* partition_register_table_descriptor(const partition_table_descriptor_config_t* config);



void partition_unregister_table_descriptor(partition_table_descriptor_t* descriptor);



void partition_load_from_drive(drive_t* drive);



partition_t* partition_create(drive_t* drive,u32 index,const char* name,u64 start_lba,u64 end_lba,const u8* guid);



#endif
