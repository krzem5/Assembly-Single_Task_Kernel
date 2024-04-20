#include <kernel/config/config.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "user_database"



#define USER_DATABASE_FILE "/etc/users.config"



static void _load_user_database(void){
	vfs_node_t* node=vfs_lookup(NULL,USER_DATABASE_FILE,0,0,0);
	if (!node){
		return;
	}
	node->uid=0;
	node->gid=0;
	node->flags&=~VFS_NODE_PERMISSION_MASK;
	node->flags|=(0000<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	config_tag_t* root_tag=config_load_from_file(node,NULL);
	if (!root_tag){
		return;
	}
	ERROR("_load_user_database: Unimplemented");
	config_tag_delete(root_tag);
}



MODULE_INIT(){
	LOG("Loading user database...");
	_load_user_database();
}
