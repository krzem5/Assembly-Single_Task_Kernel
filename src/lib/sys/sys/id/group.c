#include <sys/id/group.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_gid_t sys_gid_get(void){
	return _sys_syscall_gid_get();
}



SYS_PUBLIC sys_error_t sys_gid_set(sys_gid_t gid){
	return _sys_syscall_gid_set(gid);
}



SYS_PUBLIC sys_error_t sys_gid_get_name(sys_gid_t gid,char* name,u32 size){
	return _sys_syscall_gid_get_name(gid,name,size);
}
