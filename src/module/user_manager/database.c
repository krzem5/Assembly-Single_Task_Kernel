#include <kernel/config/config.h>
#include <kernel/error/error.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "user_database"



#define USER_DATABASE_FILE "/etc/users.config"
#define USER_DATABASE_PASSWORD "password"

#define USER_DATABASE_FIRST_USER_ID 1000



typedef struct _USER_DATABASE_ENTRY{
	rb_tree_node_t rb_node;
	u32 has_password;
	u32 password_hash[8];
} user_database_entry_t;



typedef struct _USER_DATABASE{
	spinlock_t lock;
	rb_tree_t tree;
} user_database_t;



static omm_allocator_t* KERNEL_INIT_WRITE _user_database_entry_allocator=NULL;
static user_database_t _user_database;



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
	config_tag_t* root_tag=config_load_from_file(node,USER_DATABASE_PASSWORD);
	if (!root_tag){
		return;
	}
	ERROR("_load_user_database: Unimplemented");
	config_tag_delete(root_tag);
}



error_t user_database_create_user(uid_t uid,const char* name,const char* password,u32 password_length){
	spinlock_acquire_exclusive(&(_user_database.lock));
	_Bool generate_uid=!uid;
	if (generate_uid){
		uid=USER_DATABASE_FIRST_USER_ID;
_regenerate_uid:
		for (;rb_tree_lookup_node(&(_user_database.tree),uid);uid++);
	}
	else if (rb_tree_lookup_node(&(_user_database.tree),uid)){
		spinlock_release_exclusive(&(_user_database.lock));
		return ERROR_ALREADY_PRESENT;
	}
	error_t err=uid_create(uid,name);
	if (err==ERROR_ALREADY_PRESENT){
		if (generate_uid){
			uid++;
			goto _regenerate_uid;
		}
		spinlock_release_exclusive(&(_user_database.lock));
		return ERROR_ALREADY_PRESENT;
	}
	if (err!=ERROR_OK){
		spinlock_release_exclusive(&(_user_database.lock));
		return err;
	}
	user_database_entry_t* entry=omm_alloc(_user_database_entry_allocator);
	entry->rb_node.key=uid;
	entry->has_password=0;
	spinlock_release_exclusive(&(_user_database.lock));
	return uid;
}



MODULE_INIT(){
	_user_database_entry_allocator=omm_init("user_database_entry",sizeof(user_database_entry_t),8,1,pmm_alloc_counter("omm_user_database_entry"));
	spinlock_init(&(_user_database.lock));
	rb_tree_init(&(_user_database.tree));
}



MODULE_POSTINIT(){
	LOG("Loading user database...");
	_load_user_database();
	WARN("UID=%u",user_database_create_user(0,"user",NULL,0));
}
