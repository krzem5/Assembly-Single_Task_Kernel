#include <kernel/config/config.h>
#include <kernel/error/error.h>
#include <kernel/hash/sha256.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/module/module.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "user_database"



#define USER_DATABASE_FILE "/etc/users.config"
#define USER_DATABASE_PASSWORD "password"

#define USER_DATABASE_FIRST_USER_ID 1000

#define USER_DATABASE_ENTRY_FLAG_HAS_PASSWORD 1



typedef struct _USER_DATABASE_ENTRY{
	rb_tree_node_t rb_node;
	u32 flags;
	u32 password_hash[8];
} user_database_entry_t;



typedef struct _USER_DATABASE{
	spinlock_t lock;
	rb_tree_t tree;
} user_database_t;



static omm_allocator_t* KERNEL_INIT_WRITE _user_database_entry_allocator=NULL;
static user_database_t _user_database;



static void KERNEL_EARLY_EXEC _load_database_config(config_tag_t* root_tag){
	config_tag_t* user_tag;
	for (u64 pointer=config_tag_find(root_tag,"user",0,&user_tag);pointer;pointer=config_tag_find(root_tag,"user",pointer,&user_tag)){
		if (user_tag->type!=CONFIG_TAG_TYPE_ARRAY){
			continue;
		}
		config_tag_t* id_tag;
		if (!config_tag_find(user_tag,"id",0,&id_tag)||id_tag->type!=CONFIG_TAG_TYPE_INT){
			continue;
		}
		config_tag_t* flags_tag;
		if (!config_tag_find(user_tag,"flags",0,&flags_tag)||flags_tag->type!=CONFIG_TAG_TYPE_INT){
			continue;
		}
		config_tag_t* name_tag;
		if (!config_tag_find(user_tag,"name",0,&name_tag)||name_tag->type!=CONFIG_TAG_TYPE_STRING){
			continue;
		}
		config_tag_t* password_hash_tag=NULL;
		if (flags_tag->int_&USER_DATABASE_ENTRY_FLAG_HAS_PASSWORD){
			if (!config_tag_find(user_tag,"password_hash",0,&password_hash_tag)||password_hash_tag->type!=CONFIG_TAG_TYPE_STRING||password_hash_tag->string->length!=8*sizeof(u32)){
				continue;
			}
		}
		if (uid_create(id_tag->int_,name_tag->string->data)!=ERROR_OK){
			ERROR("Unable to create user '%u:%s'",id_tag->int_,name_tag->string->data);
			continue;
		}
		user_database_entry_t* entry=omm_alloc(_user_database_entry_allocator);
		entry->rb_node.key=id_tag->int_;
		entry->flags=flags_tag->int_;
		if (entry->flags&USER_DATABASE_ENTRY_FLAG_HAS_PASSWORD){
			mem_copy(entry->password_hash,password_hash_tag->string->data,8*sizeof(u32));
		}
		rb_tree_insert_node(&(_user_database.tree),&(entry->rb_node));
	}
}



static void KERNEL_EARLY_EXEC _load_user_database(void){
	LOG("Loading user database...");
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
	_load_database_config(root_tag);
	config_tag_delete(root_tag);
}



static config_tag_t* _generate_database_config(void){
	config_tag_t* root_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"");
	for (const user_database_entry_t* entry=(void*)rb_tree_iter_start(&(_user_database.tree));entry;entry=(void*)rb_tree_iter_next(&(_user_database.tree),(void*)entry)){
		config_tag_t* user_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"user");
		config_tag_attach(root_tag,user_tag);
		config_tag_t* id_tag=config_tag_create(CONFIG_TAG_TYPE_INT,"id");
		config_tag_attach(user_tag,id_tag);
		id_tag->int_=entry->rb_node.key;
		config_tag_t* flags_tag=config_tag_create(CONFIG_TAG_TYPE_INT,"flags");
		config_tag_attach(user_tag,flags_tag);
		flags_tag->int_=entry->flags;
		config_tag_t* name_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"name");
		config_tag_attach(user_tag,name_tag);
		smm_dealloc(name_tag->string);
		char buffer[256];
		uid_get_name(entry->rb_node.key,buffer,sizeof(buffer));
		name_tag->string=smm_alloc(buffer,0);
		if (entry->flags&USER_DATABASE_ENTRY_FLAG_HAS_PASSWORD){
			config_tag_t* password_hash_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"password_hash");
			config_tag_attach(user_tag,password_hash_tag);
			smm_dealloc(password_hash_tag->string);
			password_hash_tag->string=smm_alloc((void*)(entry->password_hash),8*sizeof(u32));
		}
	}
	return root_tag;
}



static void _save_user_database(void){
	LOG("Saving user database...");
	config_tag_t* root_tag=_generate_database_config();
	vfs_node_t* parent;
	const char* child_name;
	vfs_node_t* node=vfs_lookup_for_creation(NULL,USER_DATABASE_FILE,0,0,0,&parent,&child_name);
	if (!node){
		SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
		node=vfs_node_create(NULL,parent,child_name_string,VFS_NODE_TYPE_FILE|VFS_NODE_FLAG_CREATE);
		if (!node){
			ERROR("Unable to create user database file");
			return;
		}
	}
	node->uid=0;
	node->gid=0;
	node->flags&=~VFS_NODE_PERMISSION_MASK;
	node->flags|=(0000<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	config_save_to_file(root_tag,node,USER_DATABASE_PASSWORD,0);
	config_tag_delete(root_tag);
}



KERNEL_PUBLIC error_t user_database_create_user(uid_t uid,const char* name,const char* password,u32 password_length){
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
	entry->flags=0;
	if (password_length){
		hash_sha256_state_t state;
		hash_sha256_init(&state);
		hash_sha256_process_chunk(&state,password,password_length);
		hash_sha256_finalize(&state);
		entry->flags|=USER_DATABASE_ENTRY_FLAG_HAS_PASSWORD;
		mem_copy(entry->password_hash,state.result,8*sizeof(u32));
	}
	rb_tree_insert_node(&(_user_database.tree),&(entry->rb_node));
	_save_user_database();
	spinlock_release_exclusive(&(_user_database.lock));
	return uid;
}



MODULE_INIT(){
	_user_database_entry_allocator=omm_init("user_database_entry",sizeof(user_database_entry_t),8,1);
	spinlock_init(&(_user_database.lock));
	rb_tree_init(&(_user_database.tree));
}



MODULE_POSTINIT(){
	_load_user_database();
	// WARN("UID=%u",user_database_create_user(0,"user","abc",3));
}
