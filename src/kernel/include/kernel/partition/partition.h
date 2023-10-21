#ifndef _KERNEL_PARTITION_PARTITION_H_
#define _KERNEL_PARTITION_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>



typedef struct _PARTITION{
	handle_t handle;
	const partition_descriptor_t* partition_descriptor;
	drive_t* drive;
	char name[32];
	u64 start_lba;
	u64 end_lba;
	filesystem_t* fs;
} partition_t;



extern handle_type_t HANDLE_TYPE_PARTITION;
extern handle_type_t HANDLE_TYPE_PARTITION_DESCRIPTOR;



void partition_register_descriptor(partition_descriptor_t* descriptor);



void partition_unregister_descriptor(partition_descriptor_t* descriptor);



void partition_load_from_drive(drive_t* drive);



partition_t* partition_create(drive_t* drive,const char* name,u64 start_lba,u64 end_lba);



#endif
