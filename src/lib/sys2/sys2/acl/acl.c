#include <sys2/acl/acl.h>
#include <sys2/error/error.h>
#include <sys2/mp/process.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_acl_permission_flags_t sys2_acl_get_permissions(sys2_handle_t handle,sys2_process_t process){
	return _sys2_syscall_acl_get_permissions(handle,process);
}



SYS2_PUBLIC sys2_error_t sys2_acl_request_permissions(sys2_handle_t handle,sys2_process_t process,sys2_acl_permission_flags_t permission_flags){
	return _sys2_syscall_acl_request_permissions(handle,process,permission_flags);
}



SYS2_PUBLIC sys2_error_t sys2_acl_set_permissions(sys2_handle_t handle,sys2_process_t process,sys2_acl_permission_flags_t clear,sys2_acl_permission_flags_t set){
	return _sys2_syscall_acl_set_permissions(handle,process,clear,set);
}
