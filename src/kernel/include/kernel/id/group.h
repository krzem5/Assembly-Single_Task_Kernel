#ifndef _KERNEL_ID_GROUP_H_
#define _KERNEL_ID_GROUP_H_ 1
#include <kernel/types.h>



typedef u32 gid_t;



void gid_init(void);



_Bool gid_create(gid_t gid,const char* name);



#endif
