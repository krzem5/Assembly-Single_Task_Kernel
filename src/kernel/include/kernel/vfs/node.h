#ifndef _KERNEL_VFS_NODE_H_
#define _KERNEL_VFS_NODE_H_ 1
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>



#define VFS_NODE_TYPE_MASK 7
#define VFS_NODE_TYPE_UNKNOWN 0
#define VFS_NODE_TYPE_FILE 1
#define VFS_NODE_TYPE_DIRECTORY 2
#define VFS_NODE_TYPE_LINK 3
#define VFS_NODE_TYPE_PIPE 4
#define VFS_NODE_TYPE_SOCKET 5

#define VFS_NODE_FLAG_DIRTY 8
#define VFS_NODE_FLAG_VIRTUAL 16
#define VFS_NODE_FLAG_PERMANENT 32
#define VFS_NODE_FLAG_RESIZE_RELATIVE 64
#define VFS_NODE_FLAG_NONBLOCKING 128
#define VFS_NODE_FLAG_PIPE_PEEK 256

#define VFS_NODE_PERMISSION_MASK 0x3fe00
#define VFS_NODE_PERMISSION_SHIFT 9



typedef struct _VFS_FUNCTIONS{
	struct _VFS_NODE* (*create)(struct _VFS_NODE*,const string_t*,u32);
	void (*delete)(struct _VFS_NODE*);
	struct _VFS_NODE* (*lookup)(struct _VFS_NODE*,const string_t*);
	u64 (*iterate)(struct _VFS_NODE*,u64,string_t**);
	_Bool (*link)(struct _VFS_NODE*,struct _VFS_NODE*);
	_Bool (*unlink)(struct _VFS_NODE*);
	u64 (*read)(struct _VFS_NODE*,u64,void*,u64,u32);
	u64 (*write)(struct _VFS_NODE*,u64,const void*,u64,u32);
	u64 (*resize)(struct _VFS_NODE*,s64,u32);
	void (*flush)(struct _VFS_NODE*);
} vfs_functions_t;



typedef struct _VFS_NODE_RELATIVES{
	struct _VFS_NODE* parent;
	struct _VFS_NODE* prev_sibling;
	struct _VFS_NODE* next_sibling;
	struct _VFS_NODE* child;
} vfs_node_relatives_t;



typedef struct _VFS_NODE{
	spinlock_t lock;
	u32 flags;
	KERNEL_ATOMIC u64 rc;
	string_t* name;
	vfs_node_relatives_t relatives;
	struct _FILESYSTEM* fs;
	const vfs_functions_t* functions;
	u64 time_access;
	u64 time_modify;
	u64 time_change;
	u64 time_birth;
	gid_t gid;
	uid_t uid;
} vfs_node_t;



vfs_node_t* vfs_node_create(struct _FILESYSTEM* fs,const string_t* name);



vfs_node_t* vfs_node_create_virtual(vfs_node_t* parent,const vfs_functions_t* functions,const string_t* name);



void vfs_node_delete(vfs_node_t* node);



vfs_node_t* vfs_node_lookup(vfs_node_t* node,const string_t* name);



u64 vfs_node_iterate(vfs_node_t* node,u64 pointer,string_t** out);



_Bool vfs_node_link(vfs_node_t* node,vfs_node_t* parent);



_Bool vfs_node_unlink(vfs_node_t* node);



u64 vfs_node_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags);



u64 vfs_node_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags);



u64 vfs_node_resize(vfs_node_t* node,s64 offset,u32 flags);



void vfs_node_flush(vfs_node_t* node);



void vfs_node_attach_child(vfs_node_t* node,vfs_node_t* child);



void vfs_node_dettach_child(vfs_node_t* node);



#endif
