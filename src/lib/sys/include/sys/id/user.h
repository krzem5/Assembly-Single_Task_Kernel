#ifndef _SYS_ID_USER_H_
#define _SYS_ID_USER_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u32 sys_uid_t;



sys_uid_t sys_uid_get(void);



sys_error_t sys_uid_set(sys_uid_t uid);



sys_error_t __attribute__((access(write_only,2,3),nonnull)) sys_uid_get_name(sys_uid_t uid,char* name,u32 size);



#endif
