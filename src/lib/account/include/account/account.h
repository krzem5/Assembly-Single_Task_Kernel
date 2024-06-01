#ifndef _ACCOUNT_ACCOUNT_H_
#define _ACCOUNT_ACCOUNT_H_ 1
#include <sys/error/error.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/mp/process.h>
#include <sys/types.h>



#define ACCOUNT_USER_FLAG_HAS_PASSWORD 1
#define ACCOUNT_USER_FLAG_IS_ADMINISTRATOR 2



typedef struct _ACCOUNT_GROUP{
	sys_gid_t gid;
} account_group_t;



typedef struct _ACCOUNT_USER{
	sys_uid_t uid;
	u32 flags;
} account_user_t;



sys_gid_t account_iter_group_start(void);



sys_gid_t account_iter_group_next(sys_gid_t gid);



sys_uid_t account_iter_user_start(void);



sys_uid_t account_iter_user_next(sys_uid_t uid);



sys_uid_t account_iter_user_group_start(sys_uid_t uid);



sys_uid_t account_iter_user_group_next(sys_uid_t uid,sys_gid_t gid);



sys_error_t account_get_group_data(sys_gid_t gid,account_group_t* out);



sys_error_t account_get_user_data(sys_uid_t uid,account_user_t* out);



sys_error_t account_create_group(sys_gid_t gid,const char* name);



sys_error_t account_create_user(sys_uid_t uid,const char* name,const void* password,u32 password_length);



sys_error_t account_delete_group(sys_gid_t gid);



sys_error_t account_delete_user(sys_uid_t uid);



sys_error_t account_join_group(sys_uid_t uid,sys_gid_t gid);



sys_error_t account_leave_group(sys_uid_t uid,sys_gid_t gid);



sys_error_t account_switch_user(sys_process_t process,sys_uid_t uid,const void* password,u32 password_length);



sys_error_t account_set_administrator(sys_uid_t uid,bool enable);



#endif
