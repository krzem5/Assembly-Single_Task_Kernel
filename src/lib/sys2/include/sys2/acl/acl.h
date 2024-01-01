#ifndef _SYS2_ACL_ACL_H_
#define _SYS2_ACL_ACL_H_ 1
#include <sys2/error/error.h>
#include <sys2/mp/process.h>
#include <sys2/types.h>



typedef u64 sys2_handle_t;



typedef u64 sys2_acl_permission_flags_t;



sys2_acl_permission_flags_t sys2_acl_get_permissions(sys2_handle_t handle,sys2_process_t process);



sys2_error_t sys2_acl_request_permissions(sys2_handle_t handle,sys2_process_t process,sys2_acl_permission_flags_t permission_flags);



sys2_error_t sys2_acl_set_permissions(sys2_handle_t handle,sys2_process_t process,sys2_acl_permission_flags_t clear,sys2_acl_permission_flags_t set);



#endif
