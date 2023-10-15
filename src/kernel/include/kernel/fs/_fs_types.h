#ifndef _KERNEL_FS__FS_TYPES_H_
#define _KERNEL_FS__FS_TYPES_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



typedef u16 filesystem_type_t;



typedef struct _FILESYSTEM_NODE{
	lock_t lock;
} filesystem_node_t;



typedef struct _FILESYSTEM2{
	handle_t handle;
	lock_t lock;
	filesystem_type_t type;
	filesystem_node_t* root;
} filesystem2_t;



#endif
