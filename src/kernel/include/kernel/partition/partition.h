#ifndef _KERNEL_PARTITION_PARTITION_H_
#define _KERNEL_PARTITION_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>
#include <kernel/vfs/allocator.h>



#define PARTITION_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO 1

#define PARTITION_FLAG_BOOT 1
#define PARTITION_FLAG_HALF_INSTALLED 2
#define PARTITION_FLAG_PREVIOUS_BOOT 4

#define PARTITION_CONFIG_TYPE_DRIVE 0
#define PARTITION_CONFIG_TYPE_EMPTY 1
#define PARTITION_CONFIG_TYPE_ISO9660 2
#define PARTITION_CONFIG_TYPE_KFS 3

#define PARTITION_TYPE_UNKNOWN 0

#define PARTITION_DECLARE_TYPE(name,load_code) \
	partition_type_t PARTITION_TYPE_##name; \
	static _Bool _partition_load_callback_##name(drive2_t* drive){load_code;} \
	static const partition_descriptor_t _partition_descriptor_##name={ \
		#name, \
		&(PARTITION_TYPE_##name), \
		_partition_load_callback_##name \
	}; \
	static const partition_descriptor_t*const __attribute__((used,section(".partition"))) _partition_descriptor_ptr_##name=&_partition_descriptor_##name;



typedef struct _PARTITION_DESCRIPTOR{
	const char* name;
	partition_type_t* var;
	_Bool (*load_callback)(drive2_t*);
} partition_descriptor_t;



typedef struct _PARTITION2{
	handle_t handle;
	drive2_t* drive;
	char name[32];
	u64 start_lba;
	u64 end_lba;
	filesystem2_t* fs;
} partition2_t;



extern handle_type_t HANDLE_TYPE_PARTITION;



void partition_init(void);



void partition_load_from_drive(drive2_t* drive);



partition2_t* partition_create(drive2_t* drive,const char* name,u64 start_lba,u64 end_lba);



#endif
