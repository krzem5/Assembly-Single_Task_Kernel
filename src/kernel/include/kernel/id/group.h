#ifndef _KERNEL_ID_GROUP_H_
#define _KERNEL_ID_GROUP_H_ 1
#include <kernel/error/error.h>
#include <kernel/id/flags.h>
#include <kernel/types.h>



typedef u32 gid_t;



error_t gid_create(gid_t gid,const char* name);



error_t gid_get_name(gid_t gid,char* buffer,u32 buffer_length);



id_flags_t gid_get_flags(gid_t gid);



#endif
