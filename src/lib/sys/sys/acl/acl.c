#include <sys/acl/acl.h>
#include <sys/error/error.h>
#include <sys/mp/process.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_acl_permission_flags_t sys_acl_get_permissions(sys_handle_t handle,sys_process_t process){
	return _sys_syscall_acl_get_permissions(handle,process);
}



SYS_PUBLIC sys_error_t sys_acl_request_permissions(sys_handle_t handle,sys_process_t process,sys_acl_permission_flags_t permission_flags){
	return _sys_syscall_acl_request_permissions(handle,process,permission_flags);
}



SYS_PUBLIC sys_error_t sys_acl_set_permissions(sys_handle_t handle,sys_process_t process,sys_acl_permission_flags_t clear,sys_acl_permission_flags_t set){
	return _sys_syscall_acl_set_permissions(handle,process,clear,set);
}
