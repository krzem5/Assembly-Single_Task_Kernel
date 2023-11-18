#ifndef _KERNEL_ID_USER_H_
#define _KERNEL_ID_USER_H_ 1
#include <kernel/id/group.h>
#include <kernel/types.h>



typedef u32 uid_t;



void uid_init(void);



_Bool uid_create(uid_t uid,const char* name);



_Bool uid_add_group(uid_t uid,gid_t gid);



#endif
