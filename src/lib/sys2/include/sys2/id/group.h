#ifndef _SYS2_ID_GROUP_H_
#define _SYS2_ID_GROUP_H_ 1
#include <sys2/error/error.h>
#include <sys2/types.h>



typedef u32 sys2_gid_t;



sys2_gid_t sys2_gid_get(void);



sys2_error_t sys2_gid_set(sys2_gid_t gid);



sys2_error_t sys2_gid_get_name(sys2_gid_t gid,char* name,u32 size);



#endif
