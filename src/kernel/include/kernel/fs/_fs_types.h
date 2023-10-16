#ifndef _KERNEL_FS__FS_TYPES_H_
#define _KERNEL_FS__FS_TYPES_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>
#include <kernel/vfs2/node.h>



typedef u16 filesystem_type_t;



typedef struct _FILESYSTEM2{
	handle_t handle;
	lock_t lock;
	filesystem_type_t type;
	vfs2_functions_t* functions;
	vfs2_node_t* root;
} filesystem2_t;



#endif
