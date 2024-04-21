#ifndef _ACCOUNT_MANAGER_DATABASE_H_
#define _ACCOUNT_MANAGER_DATABASE_H_ 1
#include <kernel/error/error.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>



#define ACCOUNT_MANAGER_DATABASE_FIRST_USER_ID 1000

#define ACCOUNT_MANAGER_DATABASE_USER_ENTRY_FLAG_HAS_PASSWORD 1



typedef struct _ACCOUNT_MANAGER_DATABASE_GROUP_ENTRY{
	rb_tree_node_t rb_node;
} account_manager_database_group_entry_t;



typedef struct _ACCOUNT_MANAGER_DATABASE_USER_ENTRY{
	rb_tree_node_t rb_node;
	u32 flags;
	u32 password_hash[8];
} account_manager_database_user_entry_t;



typedef struct _ACCOUNT_MANAGER_DATABASE{
	spinlock_t lock;
	rb_tree_t group_tree;
	rb_tree_t user_tree;
} account_manager_database_t;



typedef struct _ACCOUNT_MANAGER_DATABASE_GROUP_DATA{
	gid_t gid;
} account_manager_database_group_data_t;



typedef struct _ACCOUNT_MANAGER_DATABASE_USER_DATA{
	uid_t uid;
	u32 flags;
} account_manager_database_user_data_t;



error_t account_manager_database_create_group(gid_t gid,const char* name);



error_t account_manager_database_create_user(uid_t uid,const char* name,const char* password,u32 password_length);



error_t account_manager_database_get_group_data(uid_t uid,account_manager_database_group_data_t* out);



error_t account_manager_database_get_user_data(uid_t uid,account_manager_database_user_data_t* out);



error_t account_manager_database_iter_next_group(gid_t gid);



error_t account_manager_database_iter_next_user(uid_t uid);



error_t account_manager_database_iter_next_user_subgroup(uid_t uid,gid_t gid);



#endif
