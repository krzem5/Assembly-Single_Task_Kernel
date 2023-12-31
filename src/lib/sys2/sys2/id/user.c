#include <sys2/id/user.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



sys2_uid_t sys2_uid_get(void){
	return _sys2_syscall_uid_get();
}



sys2_error_t sys2_uid_set(sys2_uid_t uid){
	return _sys2_syscall_uid_set(uid);
}



sys2_error_t sys2_uid_get_name(sys2_uid_t uid,char* name,u32 size){
	return _sys2_syscall_uid_get_name(uid,name,size);
}
