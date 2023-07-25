#ifndef _KERNEL_FS_FS_H_
#define _KERNEL_FS_FS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/node_allocator.h>
#include <kernel/fs/partition.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define FS_MAX_FILE_SYSTEMS 64
#define FS_INVALID_FILE_SYSTEM_INDEX FS_MAX_FILE_SYSTEMS

#define FS_NODE_RELATIVE_PARENT 0
#define FS_NODE_RELATIVE_PREV_SIBLING 1
#define FS_NODE_RELATIVE_NEXT_SIBLING 2
#define FS_NODE_RELATIVE_FIRST_CHILD 3



typedef struct _FS_FILE_SYSTEM_CONFIG{
	u8 node_size;
	fs_node_t* (*get_relative)(drive_t*,fs_node_t*,u8);
	_Bool (*set_relative)(drive_t*,fs_node_t*,u8,fs_node_t*);
	u64 (*read)(drive_t*,fs_node_t*,u64,u8*,u64);
	u64 (*write)(drive_t*,fs_node_t*,u64,const u8*,u64);
} fs_file_system_config_t;



typedef struct _FS_FILE_SYSTEM{
	lock_t lock;
	const fs_file_system_config_t* config;
	fs_partition_config_t partition_config;
	u8 index;
	u8 name_length;
	char name[16];
	drive_t* drive;
	fs_node_t* root;
	fs_node_allocator_t allocator;
} fs_file_system_t;



void fs_init(void);



void* fs_create_file_system(drive_t* drive,const fs_partition_config_t* partition_config,const fs_file_system_config_t* config);



u8 fs_get_file_system_count(void);



const fs_file_system_t* fs_get_file_system(u8 fs_index);



u8 fs_get_boot_file_system(void);



void fs_set_boot_file_system(u8 fs_index);



void* fs_alloc_node(u8 fs_index,const char* name,u8 name_length);



fs_node_t* fs_get_node_by_id(fs_node_id_t id);



fs_node_t* fs_get_node(fs_node_t* root,const char* path);



fs_node_t* fs_get_node_relative(fs_node_t* node,u8 relative);



fs_node_t* fs_get_node_child(fs_node_t* parent,const char* name,u8 name_length);



u64 fs_read(fs_node_t* node,u64 offset,void* buffer,u64 count);



u64 fs_write(fs_node_t* node,u64 offset,const void* buffer,u64 count);



#endif
