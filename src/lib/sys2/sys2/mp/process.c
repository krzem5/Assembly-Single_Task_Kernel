#include <sys2/error/error.h>
#include <sys2/mp/process.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_error_t sys2_process_get_termination_event(sys2_process_t process){
	return _sys2_syscall_process_get_event(process);
}



SYS2_PUBLIC sys2_process_t sys2_process_get_handle(void){
	return _sys2_syscall_process_get_pid();
}



SYS2_PUBLIC sys2_error_t sys2_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags){
	return _sys2_syscall_process_start(path,argc,argv,environ,flags);
}
