#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



u8 vfs_permissions_get(vfs_node_t* node,uid_t uid,gid_t gid){
	u16 permissions=(node->flags&VFS_NODE_PERMISSION_MASK)>>VFS_NODE_PERMISSION_SHIFT;
	if (!uid||!gid){
		permissions|=permissions>>6;
	}
	if (node->uid==uid||node->gid==gid||uid_has_group(uid,node->gid)){
		permissions|=permissions>>3;
	}
	return permissions&7;
}
