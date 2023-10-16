#ifndef _KERNEL_VFS_NAME_H_
#define _KERNEL_VFS_NAME_H_ 1
#include <kernel/types.h>



typedef struct _VFS2_NODE_NAME{
	u32 length;
	u32 hash;
	char data[];
} vfs2_node_name_t;



vfs2_node_name_t* vfs2_name_alloc(const char* name,u32 length);



void vfs2_name_dealloc(vfs2_node_name_t* name);



#endif
