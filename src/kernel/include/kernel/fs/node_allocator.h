#ifndef _KERNEL_FS_NODE_ALLOCATOR_H_
#define _KERNEL_FS_NODE_ALLOCATOR_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define FS_NODE_ALLOCATOR_SIZE_SHIFT 8

#define FS_NODE_TYPE_INVALID 0
#define FS_NODE_TYPE_FILE 1
#define FS_NODE_TYPE_DIRECTORY 2

#define FS_NODE_FLAG_ROOT 1

#define FS_NODE_ID_EMPTY 0xfffffffffffffffeull
#define FS_NODE_ID_UNKNOWN 0xffffffffffffffffull



typedef u64 fs_node_id_t;



typedef u16 fs_node_allocator_index_t;



typedef struct _FS_NODE{
	fs_node_id_t id;
	u8 type;
	u8 fs_index;
	u8 name_length;
	u8 flags;
	u8 _padding[4];
	char name[64];
	fs_node_id_t parent;
	fs_node_id_t prev_sibling;
	fs_node_id_t next_sibling;
	fs_node_id_t first_child;
} fs_node_t;



typedef struct _FS_NODE_ALLOCATOR_ENTRY{
	fs_node_id_t id;
	fs_node_t* node;
	fs_node_allocator_index_t id_at_index;
	fs_node_allocator_index_t prev;
	fs_node_allocator_index_t next;
} fs_node_allocator_entry_t;



typedef struct _FS_NODE_ALLOCATOR{
	lock_t lock;
	u8 fs_index;
	fs_node_allocator_index_t first;
	fs_node_allocator_index_t last;
	fs_node_id_t next_id;
	fs_node_allocator_entry_t* data;
	fs_node_t* root_node;
} fs_node_allocator_t;



void fs_node_allocator_init(u8 fs_index,u8 node_size,fs_node_allocator_t* out);



fs_node_t* fs_node_allocator_get(fs_node_allocator_t* allocator,fs_node_id_t id,_Bool allocate_if_not_present);



#endif
