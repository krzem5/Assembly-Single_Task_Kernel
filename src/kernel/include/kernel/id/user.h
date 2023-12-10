#ifndef _KERNEL_ID_USER_H_
#define _KERNEL_ID_USER_H_ 1
#include <kernel/id/group.h>
#include <kernel/types.h>



typedef u32 uid_t;



_Bool uid_create(uid_t uid,const char* name);



_Bool uid_add_group(uid_t uid,gid_t gid);



_Bool uid_has_group(uid_t uid,gid_t gid);



_Bool uid_get_name(uid_t uid,char* buffer,u32 buffer_length);



#endif
