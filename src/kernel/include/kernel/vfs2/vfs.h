#ifndef _KERNEL_VFS2_VFS_H_
#define _KERNEL_VFS2_VFS_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/vfs2/node.h>



void vfs2_mount(filesystem_t* fs,const char* path);



vfs2_node_t* vfs2_lookup(vfs2_node_t* root,const char* path);



#endif
