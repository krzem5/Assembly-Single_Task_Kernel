#ifndef _KERNEL_ID_USER_H_
#define _KERNEL_ID_USER_H_ 1
#include <kernel/error/error.h>
#include <kernel/id/flags.h>
#include <kernel/id/group.h>
#include <kernel/types.h>



typedef u32 uid_t;



error_t uid_create(uid_t uid,const char* name);



error_t uid_delete(uid_t uid);



error_t uid_add_group(uid_t uid,gid_t gid);



error_t uid_has_group(uid_t uid,gid_t gid);



error_t uid_remove_group(uid_t uid,gid_t gid);



error_t uid_get_name(uid_t uid,char* buffer,u32 buffer_length);



id_flags_t uid_get_flags(uid_t uid);



#endif
