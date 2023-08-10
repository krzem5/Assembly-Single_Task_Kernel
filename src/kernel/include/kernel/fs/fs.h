#ifndef _KERNEL_FS_FS_H_
#define _KERNEL_FS_FS_H_ 1
#include <kernel/types.h>



#define FS_NODE_TYPE_INVALID 0
#define FS_NODE_TYPE_FILE 1
#define FS_NODE_TYPE_DIRECTORY 2

#define FS_NODE_FLAG_ROOT 1

#define FS_NODE_ID_EMPTY 0xfffffffffffffffeull
#define FS_NODE_ID_UNKNOWN 0xffffffffffffffffull

#define FS_RELATIVE_PARENT 0
#define FS_RELATIVE_PREV_SIBLING 1
#define FS_RELATIVE_NEXT_SIBLING 2
#define FS_RELATIVE_FIRST_CHILD 3



typedef u64 fs_node_id_t;



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



void* fs_alloc(u8 fs_index,const char* name,u8 name_length);



_Bool fs_dealloc(fs_node_t* node);



fs_node_t* fs_get_by_id(fs_node_id_t id);



fs_node_t* fs_get_by_path(fs_node_t* root,const char* path,u8 type);



fs_node_t* fs_get_relative(fs_node_t* node,u8 relative);



_Bool fs_set_relative(fs_node_t* node,u8 relative,fs_node_t* other);



_Bool fs_move(fs_node_t* src_node,fs_node_t* dst_node);



_Bool fs_delete(fs_node_t* node);



fs_node_t* fs_get_child(fs_node_t* parent,const char* name,u8 name_length);



u64 fs_read(fs_node_t* node,u64 offset,void* buffer,u64 count);



u64 fs_write(fs_node_t* node,u64 offset,const void* buffer,u64 count);



u64 fs_get_size(fs_node_t* node);



_Bool fs_set_size(fs_node_t* node,u64 size);



u32 fs_get_full_path(fs_node_t* node,char* buffer,u32 buffer_length);



#endif
