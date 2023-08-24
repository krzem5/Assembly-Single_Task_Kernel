#ifndef _KERNEL_VFS_VFS_H_
#define _KERNEL_VFS_VFS_H_ 1
#include <kernel/types.h>



#define VFS_NODE_TYPE_INVALID 0
#define VFS_NODE_TYPE_FILE 1
#define VFS_NODE_TYPE_DIRECTORY 2

#define VFS_NODE_FLAG_ROOT 1

#define VFS_NODE_ID_EMPTY 0xfffffffffffffffeull
#define VFS_NODE_ID_UNKNOWN 0xffffffffffffffffull

#define VFS_RELATIVE_PARENT 0
#define VFS_RELATIVE_PREV_SIBLING 1
#define VFS_RELATIVE_NEXT_SIBLING 2
#define VFS_RELATIVE_FIRST_CHILD 3



struct _PARTITION;



typedef u64 vfs_node_id_t;



typedef struct _VFS_NODE{
	vfs_node_id_t id;
	u8 type;
	u8 vfs_index;
	u8 name_length;
	u8 flags;
	u8 _padding[4];
	char name[64];
	vfs_node_id_t parent;
	vfs_node_id_t prev_sibling;
	vfs_node_id_t next_sibling;
	vfs_node_id_t first_child;
} vfs_node_t;



void* vfs_alloc(struct _PARTITION* fs,const char* name,u8 name_length);



_Bool vfs_dealloc(vfs_node_t* node);



vfs_node_t* vfs_get_by_id(vfs_node_id_t id);



vfs_node_t* vfs_get_by_path(vfs_node_t* root,const char* path,u8 type);



vfs_node_t* vfs_get_relative(vfs_node_t* node,u8 relative);



_Bool vfs_set_relative(vfs_node_t* node,u8 relative,vfs_node_t* other);



_Bool vfs_move(vfs_node_t* src_node,vfs_node_t* dst_node);



_Bool vfs_delete(vfs_node_t* node);



vfs_node_t* vfs_get_child(vfs_node_t* parent,const char* name,u8 name_length);



u64 vfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 count);



u64 vfs_write(vfs_node_t* node,u64 offset,const void* buffer,u64 count);



u64 vfs_get_size(vfs_node_t* node);



_Bool vfs_set_size(vfs_node_t* node,u64 size);



u32 vfs_get_full_path(vfs_node_t* node,char* buffer,u32 buffer_length);



#endif
