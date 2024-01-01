#include <sys2/id/group.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_gid_t sys2_gid_get(void){
	return _sys2_syscall_gid_get();
}



SYS2_PUBLIC sys2_error_t sys2_gid_set(sys2_gid_t gid){
	return _sys2_syscall_gid_set(gid);
}



SYS2_PUBLIC sys2_error_t sys2_gid_get_name(sys2_gid_t gid,char* name,u32 size){
	return _sys2_syscall_gid_get_name(gid,name,size);
}
