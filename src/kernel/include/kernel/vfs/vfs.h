#ifndef _KERNEL_VFS_VFS_H_
#define _KERNEL_VFS_VFS_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/vfs/node.h>



void vfs_mount(filesystem_t* fs,const char* path);



vfs_node_t* vfs_lookup(vfs_node_t* root,const char* path,_Bool follow_links);



#endif
