#ifndef _KERNEL_PARTITION_PARTITION_H_
#define _KERNEL_PARTITION_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/allocator.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define FS_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO 1

#define FS_PARTITION_FLAG_BOOT 1
#define FS_PARTITION_FLAG_HALF_INSTALLED 2
#define FS_PARTITION_FLAG_PREVIOUS_BOOT 4

#define FS_MAX_PARTITIONS 64
#define FS_INVALID_PARTITION_INDEX FS_MAX_PARTITIONS

#define FS_PARTITION_CONFIG_TYPE_DRIVE 0
#define FS_PARTITION_CONFIG_TYPE_EMPTY 1
#define FS_PARTITION_CONFIG_TYPE_ISO9660 2
#define FS_PARTITION_CONFIG_TYPE_GPT 3
#define FS_PARTITION_CONFIG_TYPE_KFS 4



struct _FS_PARTITION;



typedef struct _FS_FILE_SYSTEM_CONFIG{
	u8 node_size;
	u8 flags;
	fs_node_t* (*create)(struct _FS_PARTITION*,_Bool,const char*,u8);
	_Bool (*delete)(struct _FS_PARTITION*,fs_node_t*);
	fs_node_t* (*get_relative)(struct _FS_PARTITION*,fs_node_t*,u8);
	_Bool (*set_relative)(struct _FS_PARTITION*,fs_node_t*,u8,fs_node_t*);
	_Bool (*move_file)(struct _FS_PARTITION*,fs_node_t*,fs_node_t*);
	u64 (*read)(struct _FS_PARTITION*,fs_node_t*,u64,u8*,u64);
	u64 (*write)(struct _FS_PARTITION*,fs_node_t*,u64,const u8*,u64);
	u64 (*get_size)(struct _FS_PARTITION*,fs_node_t*);
	void (*flush_cache)(struct _FS_PARTITION*);
} fs_file_system_config_t;



typedef struct _FS_PARTITION_CONFIG{
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
} fs_partition_config_t;



typedef struct _FS_PARTITION{
	lock_t lock;
	const fs_file_system_config_t* config;
	fs_partition_config_t partition_config;
	u8 index;
	u8 flags;
	u8 name_length;
	char name[16];
	const drive_t* drive;
	fs_node_t* root;
	fs_allocator_t allocator;
	void* extra_data;
} fs_partition_t;



extern fs_partition_t* partition_data;
extern u8 partition_count;
extern u8 partition_boot_index;



void fs_partition_init(void);



void* fs_partition_add(const drive_t* drive,const fs_partition_config_t* partition_config,const fs_file_system_config_t* config,void* extra_data);



void fs_partition_load_from_drive(drive_t* drive);



void fs_partition_flush_cache(void);



#endif
