#ifndef _KERNEL_PARTITION_PARTITION_H_
#define _KERNEL_PARTITION_PARTITION_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
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



struct _PARTITION;



typedef struct _PARTITION_FILE_SYSTEM_CONFIG{
	u8 node_size;
	u8 flags;
	vfs_node_t* (*create)(struct _PARTITION*,_Bool,const char*,u8);
	_Bool (*delete)(struct _PARTITION*,vfs_node_t*);
	vfs_node_t* (*get_relative)(struct _PARTITION*,vfs_node_t*,u8);
	_Bool (*set_relative)(struct _PARTITION*,vfs_node_t*,u8,vfs_node_t*);
	_Bool (*move_file)(struct _PARTITION*,vfs_node_t*,vfs_node_t*);
	u64 (*read)(struct _PARTITION*,vfs_node_t*,u64,u8*,u64);
	u64 (*write)(struct _PARTITION*,vfs_node_t*,u64,const u8*,u64);
	u64 (*get_size)(struct _PARTITION*,vfs_node_t*);
	_Bool (*set_size)(struct _PARTITION*,vfs_node_t*,u64);
	void (*flush_cache)(struct _PARTITION*);
} partition_file_system_config_t;



typedef struct _PARTITION_CONFIG{
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
} partition_config_t;



typedef struct _PARTITION{
	struct _PARTITION* next;
	lock_t lock;
	const partition_file_system_config_t* config;
	partition_config_t partition_config;
	u8 index;
	u8 flags;
	u8 name_length;
	char name[16];
	const drive_t* drive;
	vfs_node_t* root;
	vfs_allocator_t allocator;
	void* extra_data;
} partition_t;



typedef struct _FILESYSTEM2{
	u64 _tmp;
} filesystem2_t;



typedef struct _PARTITION2{
	handle_t handle;
	drive2_t* drive;
	char name[32];
	u64 start_lba;
	u64 end_lba;
	filesystem2_t* fs;
} partition2_t;



extern partition_t* partition_data;
extern partition_t* partition_boot;



void* partition_add(const drive_t* drive,const partition_config_t* partition_config,const partition_file_system_config_t* config,void* extra_data);



partition_t* partition_get(u32 index);



void partition_load_from_drive(drive2_t* drive);



void partition_load(void);



void partition_flush_cache(void);



#endif
