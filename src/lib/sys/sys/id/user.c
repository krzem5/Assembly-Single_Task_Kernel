#include <sys/id/user.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_uid_t sys_uid_get(void){
	return _sys_syscall_uid_get();
}



SYS_PUBLIC sys_error_t sys_uid_set(sys_uid_t uid){
	return _sys_syscall_uid_set(uid);
}



SYS_PUBLIC sys_error_t sys_uid_get_name(sys_uid_t uid,char* name,u32 size){
	return _sys_syscall_uid_get_name(uid,name,size);
}
