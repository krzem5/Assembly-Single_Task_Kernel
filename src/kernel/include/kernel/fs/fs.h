#ifndef _KERNEL_FS_FS_H_
#define _KERNEL_FS_FS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/_fs_types.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>



#define FS_DESCRIPTOR_USER_FLAG_CAN_FORMAT 1



typedef struct _FILESYSTEM_DESCRIPTOR_CONFIG{
	const char* name;
	void (*deinit_callback)(filesystem_t*);
	filesystem_t* (*load_callback)(partition_t*);
	void (*mount_callback)(filesystem_t*,const char*);
	bool (*format_callback)(partition_t*);
} filesystem_descriptor_config_t;



typedef struct _FILESYSTEM_DESCRIPTOR{
	const filesystem_descriptor_config_t* config;
	handle_t handle;
} filesystem_descriptor_t;



typedef struct _FILESYSTEM_USER_DATA{
	char type[64];
	handle_id_t partition;
	u8 uuid[16];
	char mount_path[256];
} filesystem_user_data_t;



typedef struct _FILESYSTEM_DESCRIPTOR_USER_DATA{
	char name[64];
	u32 flags;
} filesystem_descriptor_user_data_t;



extern handle_type_t fs_handle_type;
extern handle_type_t fs_descriptor_handle_type;



void fs_register_descriptor(const filesystem_descriptor_config_t* config,filesystem_descriptor_t** out);



void fs_unregister_descriptor(filesystem_descriptor_t* descriptor);



filesystem_t* fs_create(filesystem_descriptor_t* descriptor);



filesystem_t* fs_load(partition_t* partition);



bool fs_format(partition_t* partition,const filesystem_descriptor_t* descriptor);



#endif
