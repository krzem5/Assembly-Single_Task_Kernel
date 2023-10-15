#ifndef _KERNEL_FS__FS_TYPES_H_
#define _KERNEL_FS__FS_TYPES_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define VFS2_NODE_FLAG_DIRTY 1
#define VFS2_NODE_FLAG_VIRTUAL 2
#define VFS2_NODE_FLAG_PERMANENT 4
#define VFS2_NODE_FLAG_RESIZE_RELATIVE 8
#define VFS2_NODE_TYPE_SHIFT 28
#define VFS2_NODE_TYPE_UNKNOWN 0
#define VFS2_NODE_TYPE_FILE 1
#define VFS2_NODE_TYPE_DIRECTORY 2
#define VFS2_NODE_TYPE_LINK 3



typedef u16 filesystem_type_t;



// file name length groups: 8,16,24,32,48,64,96,128,192,256
typedef struct _VFS2_NODE_NAME{
	u32 length;
	u32 hash;
	char data[];
} vfs2_node_name_t;



typedef struct _VFS2_FUNCTIONS{
	struct _VFS2_NODE* (*create)(void);
	void (*delete)(struct _VFS2_NODE*);
	struct _VFS2_NODE* (*lookup)(struct _VFS2_NODE*,vfs2_node_name_t*);
	_Bool (*link)(struct _VFS2_NODE*,struct _VFS2_NODE*);
	_Bool (*unlink)(struct _VFS2_NODE*);
	s64 (*read)(struct _VFS2_NODE*,u64,void*,u64);
	s64 (*write)(struct _VFS2_NODE*,u64,void*,u64);
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



typedef struct _FILESYSTEM2{
	handle_t handle;
	lock_t lock;
	filesystem_type_t type;
} filesystem2_t;



#endif
