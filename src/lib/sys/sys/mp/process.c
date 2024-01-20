#include <sys/mp/event.h>
#include <sys/mp/process.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_event_t sys_process_get_termination_event(sys_process_t process){
	return _sys_syscall_process_get_event(process);
}



SYS_PUBLIC sys_process_t sys_process_get_handle(void){
	return _sys_syscall_process_get_pid();
}



SYS_PUBLIC sys_process_t sys_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags){
	return _sys_syscall_process_start(path,argc,argv,environ,flags);
}



SYS_PUBLIC sys_error_t sys_process_set_cwd(sys_process_t process,sys_fd_t fd){
	return _sys_syscall_process_set_cwd(process,fd);
}
