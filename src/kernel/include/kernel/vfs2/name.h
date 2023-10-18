#ifndef _KERNEL_VFS2_NAME_H_
#define _KERNEL_VFS2_NAME_H_ 1
#include <kernel/types.h>



#define VFS2_NAME_MAX_LENGTH 255



typedef struct _VFS2_NODE_NAME{
	u32 length;
	u32 hash;
	char data[];
} vfs2_name_t;



vfs2_name_t* vfs2_name_alloc(const char* name,u32 length);



void vfs2_name_dealloc(vfs2_name_t* name);



vfs2_name_t* vfs2_name_duplicate(const vfs2_name_t* name);



#endif
