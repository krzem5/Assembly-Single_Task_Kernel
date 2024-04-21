#include <account_manager/database.h>
#include <kernel/config/config.h>
#include <kernel/error/error.h>
#include <kernel/hash/sha256.h>
#include <kernel/id/group.h>
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
#define KERNEL_LOG_NAME "account_database"



#define ACCOUNT_DATABASE_FILE "/etc/accounts.config"
#define ACCOUNT_DATABASE_PASSWORD NULL//"password"



static omm_allocator_t* KERNEL_INIT_WRITE _account_database_group_entry_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _account_database_user_entry_allocator=NULL;
static account_database_t _account_database;



static void KERNEL_EARLY_EXEC _load_database_config(config_tag_t* root_tag){
	config_tag_t* groups_tag;
	if (!config_tag_find(root_tag,"groups",0,&groups_tag)||groups_tag->type!=CONFIG_TAG_TYPE_ARRAY){
		return;
	}
	config_tag_t* users_tag;
	if (!config_tag_find(root_tag,"users",0,&users_tag)||users_tag->type!=CONFIG_TAG_TYPE_ARRAY){
		return;
	}
	config_tag_t* group_tag;
	for (u64 pointer=config_tag_find(groups_tag,"group",0,&group_tag);pointer;pointer=config_tag_find(groups_tag,"group",pointer,&group_tag)){
		if (group_tag->type!=CONFIG_TAG_TYPE_ARRAY){
			continue;
		}
		config_tag_t* id_tag;
		if (!config_tag_find(group_tag,"id",0,&id_tag)||id_tag->type!=CONFIG_TAG_TYPE_INT){
			continue;
		}
		config_tag_t* name_tag;
		if (!config_tag_find(group_tag,"name",0,&name_tag)||name_tag->type!=CONFIG_TAG_TYPE_STRING){
			continue;
		}
		if (gid_create(id_tag->int_,name_tag->string->data)!=ERROR_OK){
			ERROR("Unable to create group '%u:%s'",id_tag->int_,name_tag->string->data);
			continue;
		}
		account_database_group_entry_t* entry=omm_alloc(_account_database_group_entry_allocator);
		entry->rb_node.key=id_tag->int_;
		rb_tree_insert_node(&(_account_database.group_tree),&(entry->rb_node));
	}
	config_tag_t* user_tag;
	for (u64 pointer=config_tag_find(users_tag,"user",0,&user_tag);pointer;pointer=config_tag_find(users_tag,"user",pointer,&user_tag)){
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
		if (flags_tag->int_&ACCOUNT_DATABASE_USER_ENTRY_FLAG_HAS_PASSWORD){
			if (!config_tag_find(user_tag,"password_hash",0,&password_hash_tag)||password_hash_tag->type!=CONFIG_TAG_TYPE_STRING||password_hash_tag->string->length!=8*sizeof(u32)){
				continue;
			}
		}
		if (uid_create(id_tag->int_,name_tag->string->data)!=ERROR_OK){
			ERROR("Unable to create user '%u:%s'",id_tag->int_,name_tag->string->data);
			continue;
		}
		account_database_user_entry_t* entry=omm_alloc(_account_database_user_entry_allocator);
		entry->rb_node.key=id_tag->int_;
		entry->flags=flags_tag->int_;
		if (entry->flags&ACCOUNT_DATABASE_USER_ENTRY_FLAG_HAS_PASSWORD){
			mem_copy(entry->password_hash,password_hash_tag->string->data,8*sizeof(u32));
		}
		rb_tree_insert_node(&(_account_database.user_tree),&(entry->rb_node));
	}
}



static void KERNEL_EARLY_EXEC _load_account_database(void){
	LOG("Loading account database...");
	vfs_node_t* node=vfs_lookup(NULL,ACCOUNT_DATABASE_FILE,0,0,0);
	if (!node){
		return;
	}
	node->uid=0;
	node->gid=0;
	node->flags&=~VFS_NODE_PERMISSION_MASK;
	node->flags|=(0400<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	config_tag_t* root_tag=config_load_from_file(node,ACCOUNT_DATABASE_PASSWORD);
	if (!root_tag){
		return;
	}
	_load_database_config(root_tag);
	config_tag_delete(root_tag);
}



static config_tag_t* _generate_database_config(void){
	config_tag_t* root_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"");
	config_tag_t* groups_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"groups");
	config_tag_attach(root_tag,groups_tag);
	for (const account_database_group_entry_t* entry=(void*)rb_tree_iter_start(&(_account_database.group_tree));entry;entry=(void*)rb_tree_iter_next(&(_account_database.group_tree),(void*)entry)){
		config_tag_t* group_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"group");
		config_tag_attach(groups_tag,group_tag);
		config_tag_t* id_tag=config_tag_create(CONFIG_TAG_TYPE_INT,"id");
		config_tag_attach(group_tag,id_tag);
		id_tag->int_=entry->rb_node.key;
		config_tag_t* name_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"name");
		config_tag_attach(group_tag,name_tag);
		smm_dealloc(name_tag->string);
		char buffer[256];
		gid_get_name(entry->rb_node.key,buffer,sizeof(buffer));
		name_tag->string=smm_alloc(buffer,0);
	}
	config_tag_t* users_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"users");
	config_tag_attach(root_tag,users_tag);
	for (const account_database_user_entry_t* entry=(void*)rb_tree_iter_start(&(_account_database.user_tree));entry;entry=(void*)rb_tree_iter_next(&(_account_database.user_tree),(void*)entry)){
		config_tag_t* user_tag=config_tag_create(CONFIG_TAG_TYPE_ARRAY,"user");
		config_tag_attach(users_tag,user_tag);
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
		if (entry->flags&ACCOUNT_DATABASE_USER_ENTRY_FLAG_HAS_PASSWORD){
			config_tag_t* password_hash_tag=config_tag_create(CONFIG_TAG_TYPE_STRING,"password_hash");
			config_tag_attach(user_tag,password_hash_tag);
			smm_dealloc(password_hash_tag->string);
			password_hash_tag->string=smm_alloc((void*)(entry->password_hash),8*sizeof(u32));
		}
	}
	return root_tag;
}



static void _save_account_database(void){
	LOG("Saving account database...");
	config_tag_t* root_tag=_generate_database_config();
	vfs_node_t* parent;
	const char* child_name;
	vfs_node_t* node=vfs_lookup_for_creation(NULL,ACCOUNT_DATABASE_FILE,0,0,0,&parent,&child_name);
	if (!node){
		SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
		node=vfs_node_create(NULL,parent,child_name_string,VFS_NODE_TYPE_FILE|VFS_NODE_FLAG_CREATE);
		if (!node){
			ERROR("Unable to create account database file");
			return;
		}
	}
	node->uid=0;
	node->gid=0;
	node->flags&=~VFS_NODE_PERMISSION_MASK;
	node->flags|=(0400<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
	vfs_node_flush(node);
	config_save_to_file(root_tag,node,ACCOUNT_DATABASE_PASSWORD,0);
	config_tag_delete(root_tag);
}



MODULE_INIT(){
	_account_database_group_entry_allocator=omm_init("account_database_group_entry",sizeof(account_database_group_entry_t),8,1);
	spinlock_init(&(_account_database_group_entry_allocator->lock));
	_account_database_user_entry_allocator=omm_init("account_database_user_entry",sizeof(account_database_user_entry_t),8,1);
	spinlock_init(&(_account_database_user_entry_allocator->lock));
	spinlock_init(&(_account_database.lock));
	rb_tree_init(&(_account_database.group_tree));
	rb_tree_init(&(_account_database.user_tree));
}



MODULE_POSTINIT(){
	_load_account_database();
}



KERNEL_PUBLIC error_t account_database_create_group(gid_t gid,const char* name){
	spinlock_acquire_exclusive(&(_account_database.lock));
	if (rb_tree_lookup_node(&(_account_database.group_tree),gid)){
		spinlock_release_exclusive(&(_account_database.lock));
		return ERROR_ALREADY_PRESENT;
	}
	error_t err=gid_create(gid,name);
	if (err==ERROR_ALREADY_PRESENT){
		spinlock_release_exclusive(&(_account_database.lock));
		return ERROR_ALREADY_PRESENT;
	}
	if (err!=ERROR_OK){
		spinlock_release_exclusive(&(_account_database.lock));
		return err;
	}
	account_database_group_entry_t* group_entry=omm_alloc(_account_database_group_entry_allocator);
	group_entry->rb_node.key=gid;
	rb_tree_insert_node(&(_account_database.group_tree),&(group_entry->rb_node));
	_save_account_database();
	spinlock_release_exclusive(&(_account_database.lock));
	return gid;
}



KERNEL_PUBLIC error_t account_database_create_user(uid_t uid,const char* name,const char* password,u32 password_length){
	spinlock_acquire_exclusive(&(_account_database.lock));
	_Bool generate_uid=!uid;
	if (generate_uid){
		uid=ACCOUNT_DATABASE_FIRST_USER_ID;
_regenerate_uid:
		for (;rb_tree_lookup_node(&(_account_database.group_tree),uid)||rb_tree_lookup_node(&(_account_database.user_tree),uid);uid++);
	}
	else if (rb_tree_lookup_node(&(_account_database.group_tree),uid)||rb_tree_lookup_node(&(_account_database.user_tree),uid)){
_invalid_uid:
		spinlock_release_exclusive(&(_account_database.lock));
		return ERROR_ALREADY_PRESENT;
	}
	error_t err=gid_create(uid,name);
	if (err==ERROR_ALREADY_PRESENT){
		if (generate_uid){
			uid++;
			goto _regenerate_uid;
		}
		goto _invalid_uid;
	}
	if (err!=ERROR_OK){
		spinlock_release_exclusive(&(_account_database.lock));
		return err;
	}
	err=uid_create(uid,name);
	if (err==ERROR_ALREADY_PRESENT){
		gid_delete(uid);
		if (generate_uid){
			uid++;
			goto _regenerate_uid;
		}
		goto _invalid_uid;
	}
	if (err!=ERROR_OK){
		gid_delete(uid);
		spinlock_release_exclusive(&(_account_database.lock));
		return err;
	}
	account_database_group_entry_t* group_entry=omm_alloc(_account_database_group_entry_allocator);
	group_entry->rb_node.key=uid;
	rb_tree_insert_node(&(_account_database.group_tree),&(group_entry->rb_node));
	account_database_user_entry_t* user_entry=omm_alloc(_account_database_user_entry_allocator);
	user_entry->rb_node.key=uid;
	user_entry->flags=0;
	if (password_length){
		hash_sha256_state_t state;
		hash_sha256_init(&state);
		hash_sha256_process_chunk(&state,password,password_length);
		hash_sha256_finalize(&state);
		user_entry->flags|=ACCOUNT_DATABASE_USER_ENTRY_FLAG_HAS_PASSWORD;
		mem_copy(user_entry->password_hash,state.result,8*sizeof(u32));
	}
	rb_tree_insert_node(&(_account_database.user_tree),&(user_entry->rb_node));
	_save_account_database();
	spinlock_release_exclusive(&(_account_database.lock));
	return uid;
}
