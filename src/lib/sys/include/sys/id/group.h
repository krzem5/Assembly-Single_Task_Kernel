#ifndef _SYS_ID_GROUP_H_
#define _SYS_ID_GROUP_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u32 sys_gid_t;



sys_gid_t sys_gid_get(void);



sys_error_t sys_gid_set(sys_gid_t gid);



sys_error_t __attribute__((access(write_only,2,3),nonnull)) sys_gid_get_name(sys_gid_t gid,char* name,u32 size);



#endif
