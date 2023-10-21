#ifndef _KERNEL_PARTITION_PARTITION_H_
#define _KERNEL_PARTITION_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>



#define PARTITION_TYPE_UNKNOWN 0

#define PARTITION_DECLARE_TYPE(name,load_code) \
	partition_type_t PARTITION_TYPE_##name; \
	static _Bool _partition_load_callback_##name(drive_t* drive){load_code;} \
	static const partition_descriptor_t _partition_descriptor_##name={ \
		#name, \
		&(PARTITION_TYPE_##name), \
		_partition_load_callback_##name \
	}; \
	static const partition_descriptor_t*const __attribute__((used,section(".partition"))) _partition_descriptor_ptr_##name=&_partition_descriptor_##name;



typedef struct _PARTITION_DESCRIPTOR{
	const char* name;
	partition_type_t* var;
	_Bool (*load_callback)(drive_t*);
} partition_descriptor_t;



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



void partition_init(void);



void partition_load_from_drive(drive_t* drive);



partition_t* partition_create(drive_t* drive,const char* name,u64 start_lba,u64 end_lba);



#endif
