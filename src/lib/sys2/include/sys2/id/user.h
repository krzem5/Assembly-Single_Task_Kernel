#ifndef _SYS2_ID_USER_H_
#define _SYS2_ID_USER_H_ 1
#include <sys2/error/error.h>
#include <sys2/types.h>



typedef u32 sys2_uid_t;



sys2_uid_t sys2_uid_get(void);



sys2_error_t sys2_uid_set(sys2_uid_t uid);



sys2_error_t sys2_uid_get_name(sys2_uid_t uid,char* name,u32 size);



#endif
