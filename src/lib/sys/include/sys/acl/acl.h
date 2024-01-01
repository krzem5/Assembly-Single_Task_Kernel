#ifndef _SYS_ACL_ACL_H_
#define _SYS_ACL_ACL_H_ 1
#include <sys/error/error.h>
#include <sys/mp/process.h>
#include <sys/types.h>



typedef u64 sys_handle_t;



typedef u64 sys_acl_permission_flags_t;



sys_acl_permission_flags_t sys_acl_get_permissions(sys_handle_t handle,sys_process_t process);



sys_error_t sys_acl_request_permissions(sys_handle_t handle,sys_process_t process,sys_acl_permission_flags_t permission_flags);



sys_error_t sys_acl_set_permissions(sys_handle_t handle,sys_process_t process,sys_acl_permission_flags_t clear,sys_acl_permission_flags_t set);



#endif
