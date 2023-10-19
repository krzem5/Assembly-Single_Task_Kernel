#ifndef _KERNEL_VFS_NAME_H_
#define _KERNEL_VFS_NAME_H_ 1
#include <kernel/types.h>



#define VFS_NAME_MAX_LENGTH 255



typedef struct _vfs_node_NAME{
	u32 length;
	u32 hash;
	char data[];
} vfs_name_t;



vfs_name_t* vfs_name_alloc(const char* name,u32 length);



void vfs_name_dealloc(vfs_name_t* name);



vfs_name_t* vfs_name_duplicate(const vfs_name_t* name);



#endif
