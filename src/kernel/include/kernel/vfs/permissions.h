#ifndef _KERNEL_VFS_PERMISSIONS_H_
#define _KERNEL_VFS_PERMISSIONS_H_ 1
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



#define VFS_PERMISSION_READ 4
#define VFS_PERMISSION_WRITE 2
#define VFS_PERMISSION_EXEC 1



u8 vfs_permissions_get(vfs_node_t* node,uid_t uid,gid_t gid);



#endif
