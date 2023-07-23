#ifndef _KERNEL_FS_FS_H_
#define _KERNEL_FS_FS_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>
#include <kernel/types.h>



#define FS_MAX_FILE_SYSTEMS 64
#define FS_INVALID_FILE_SYSTEM_INDEX FS_MAX_FILE_SYSTEMS

#define FS_NODE_TYPE_FILE 1
#define FS_NODE_TYPE_DIRECTORY 2

#define FS_NODE_FLAG_ROOT 1

#define FS_NODE_ID_EMPTY 0
#define FS_NODE_ID_UNKNOWN 1
#define FS_NODE_ID_FIRST_FREE (FS_NODE_ID_UNKNOWN+1)

#define FS_NODE_RELATIVE_PARENT 0
#define FS_NODE_RELATIVE_PREV_SIBLING 1
#define FS_NODE_RELATIVE_NEXT_SIBLING 2
#define FS_NODE_RELATIVE_FIRST_CHILD 3



typedef u64 fs_node_id_t;



typedef struct _FS_NODE{
	fs_node_id_t id;
	u8 type;
	u8 fs_index;
	u8 name_length;
	u8 ref_cnt;
	u8 flags;
	u8 _padding[3];
	char name[64];
	fs_node_id_t parent;
	fs_node_id_t prev_sibling;
	fs_node_id_t next_sibling;
	fs_node_id_t first_child;
} fs_node_t;



typedef struct _FS_FILE_SYSTEM_CONFIG{
	u8 node_size;
	fs_node_t* (*get_relative)(drive_t*,fs_node_t*,u8);
	_Bool (*set_relative)(drive_t*,fs_node_t*,u8,fs_node_t*);
	u64 (*read)(drive_t*,fs_node_t*,u64,u8*,u64);
	u64 (*write)(drive_t*,fs_node_t*,u64,const u8*,u64);
} fs_file_system_config_t;



typedef struct _FS_FILE_SYSTEM_NODE_ALLOCATOR{
	fs_node_id_t next_node_id;
	void* nodes;
} fs_file_system_node_allocator_t;



typedef struct _FS_FILE_SYSTEM{
	const fs_file_system_config_t* config;
	fs_partition_config_t partition_config;
	u8 name_length;
	char name[16];
	drive_t* drive;
	fs_node_t* root;
	fs_file_system_node_allocator_t allocator;
} fs_file_system_t;



void fs_init(void);



void* fs_create_file_system(drive_t* drive,const fs_partition_config_t* partition_config,const fs_file_system_config_t* config);



u8 fs_get_file_system_count(void);



const fs_file_system_t* fs_get_file_system(u8 fs_index);



u8 fs_get_boot_file_system(void);



void fs_set_boot_file_system(u8 fs_index);



void* fs_alloc_node(u8 fs_index,const char* name,u8 name_length);



fs_node_t* fs_get_node(fs_node_t* root,const char* path);



fs_node_t* fs_get_node_relative(fs_node_t* node,u8 relative);



fs_node_t* fs_get_node_child(fs_node_t* parent,const char* name,u8 name_length);



u64 fs_read(fs_node_t* node,u64 offset,void* buffer,u64 count);



u64 fs_write(fs_node_t* node,u64 offset,const void* buffer,u64 count);



#endif
