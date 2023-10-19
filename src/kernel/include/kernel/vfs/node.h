#ifndef _KERNEL_VFS_NODE_H_
#define _KERNEL_VFS_NODE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/vfs/name.h>



#define VFS_NODE_TYPE_MASK 15
#define VFS_NODE_TYPE_UNKNOWN 0
#define VFS_NODE_TYPE_FILE 1
#define VFS_NODE_TYPE_DIRECTORY 2
#define VFS_NODE_TYPE_LINK 3

#define VFS_NODE_FLAG_DIRTY 16
#define VFS_NODE_FLAG_VIRTUAL 32
#define VFS_NODE_FLAG_PERMANENT 64
#define VFS_NODE_FLAG_RESIZE_RELATIVE 128



typedef struct _vfs_FUNCTIONS{
	struct _vfs_NODE* (*create)(void);
	void (*delete)(struct _vfs_NODE*);
	struct _vfs_NODE* (*lookup)(struct _vfs_NODE*,const vfs_name_t*);
	u64 (*iterate)(struct _vfs_NODE*,u64,vfs_name_t**);
	_Bool (*link)(struct _vfs_NODE*,struct _vfs_NODE*);
	_Bool (*unlink)(struct _vfs_NODE*);
	s64 (*read)(struct _vfs_NODE*,u64,void*,u64);
	s64 (*write)(struct _vfs_NODE*,u64,const void*,u64);
	s64 (*resize)(struct _vfs_NODE*,s64,u32);
	void (*flush)(struct _vfs_NODE*);
} vfs_functions_t;



typedef struct _vfs_node_RELATIVES{
	struct _vfs_NODE* parent;
	struct _vfs_NODE* prev_sibling;
	struct _vfs_NODE* next_sibling;
	struct _vfs_NODE* child;
} vfs_node_relatives_t;



typedef struct _vfs_NODE{
	u32 flags;
	lock_t lock;
	vfs_name_t* name;
	vfs_node_relatives_t relatives;
	struct _FILESYSTEM* fs;
	vfs_functions_t* functions;
} vfs_node_t;



vfs_node_t* vfs_node_create(struct _FILESYSTEM* fs,const vfs_name_t* name);



void vfs_node_delete(vfs_node_t* node);



vfs_node_t* vfs_node_lookup(vfs_node_t* node,const vfs_name_t* name);



u64 vfs_node_iterate(vfs_node_t* node,u64 pointer,vfs_name_t** out);



_Bool vfs_node_link(vfs_node_t* node,vfs_node_t* parent);



_Bool vfs_node_unlink(vfs_node_t* node);



s64 vfs_node_read(vfs_node_t* node,u64 offset,void* buffer,u64 size);



s64 vfs_node_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size);



s64 vfs_node_resize(vfs_node_t* node,s64 offset,u32 flags);



void vfs_node_flush(vfs_node_t* node);



#endif
