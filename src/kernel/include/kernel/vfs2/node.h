#ifndef _KERNEL_VFS2_NODE_H_
#define _KERNEL_VFS2_NODE_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/vfs2/name.h>



#define VFS2_NODE_TYPE_MASK 15
#define VFS2_NODE_TYPE_UNKNOWN 0
#define VFS2_NODE_TYPE_FILE 1
#define VFS2_NODE_TYPE_DIRECTORY 2
#define VFS2_NODE_TYPE_LINK 3

#define VFS2_NODE_FLAG_DIRTY 16
#define VFS2_NODE_FLAG_VIRTUAL 32
#define VFS2_NODE_FLAG_PERMANENT 64
#define VFS2_NODE_FLAG_RESIZE_RELATIVE 128



typedef struct _VFS2_FUNCTIONS{
	struct _VFS2_NODE* (*create)(void);
	void (*delete)(struct _VFS2_NODE*);
	struct _VFS2_NODE* (*lookup)(struct _VFS2_NODE*,const vfs2_node_name_t*);
	_Bool (*link)(struct _VFS2_NODE*,struct _VFS2_NODE*);
	_Bool (*unlink)(struct _VFS2_NODE*);
	s64 (*read)(struct _VFS2_NODE*,u64,void*,u64);
	s64 (*write)(struct _VFS2_NODE*,u64,const void*,u64);
	s64 (*resize)(struct _VFS2_NODE*,s64,u32);
	void (*flush)(struct _VFS2_NODE*);
} vfs2_functions_t;



typedef struct _VFS2_NODE_RELATIVES{
	struct _VFS2_NODE* parent;
	struct _VFS2_NODE* prev_sibling;
	struct _VFS2_NODE* next_sibling;
	struct _VFS2_NODE* child;
} vfs2_node_relatives_t;



typedef struct _VFS2_NODE{
	u32 flags;
	lock_t lock;
	vfs2_node_name_t* name;
	vfs2_node_relatives_t relatives;
	struct _FILESYSTEM2* fs;
	vfs2_functions_t* functions;
} vfs2_node_t;



vfs2_node_t* vfs2_node_create(struct _FILESYSTEM2* fs,const vfs2_node_name_t* name);



vfs2_node_t* vfs2_node_get_child(vfs2_node_t* node,const vfs2_node_name_t* name);



s64 vfs2_node_read(vfs2_node_t* node,u64 offset,void* buffer,u64 size);



s64 vfs2_node_write(vfs2_node_t* node,u64 offset,const void* buffer,u64 size);



s64 vfs2_node_resize(vfs2_node_t* node,s64 offset,u32 flags);



#endif
