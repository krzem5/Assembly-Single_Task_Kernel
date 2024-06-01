#include <sys/mp/event.h>
#include <sys/mp/process.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



typedef struct _SYSCALL_PROCESS_START_EXTRA_DATA{
	u64 fd_in;
	u64 fd_out;
	u64 fd_err;
} syscall_process_start_extra_data_t;



SYS_PUBLIC sys_event_t sys_process_get_termination_event(sys_process_t process){
	return _sys_syscall_process_get_event(process);
}



SYS_PUBLIC sys_process_t sys_process_get_handle(void){
	return _sys_syscall_process_get_pid();
}



SYS_PUBLIC sys_process_t sys_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags,sys_fd_t fd_in,sys_fd_t fd_out,sys_fd_t fd_err){
	syscall_process_start_extra_data_t extra_data={
		fd_in,
		fd_out,
		fd_err
	};
	return _sys_syscall_process_start(path,argc,argv,environ,flags,&extra_data);
}



SYS_PUBLIC sys_error_t sys_process_set_cwd(sys_process_t process,sys_fd_t fd){
	return _sys_syscall_process_set_cwd(process,fd);
}



SYS_PUBLIC sys_process_t sys_process_get_parent(sys_process_t process){
	return _sys_syscall_process_get_parent(process);
}



SYS_PUBLIC sys_error_t sys_process_set_root(sys_process_t process,sys_fd_t fd){
	return _sys_syscall_process_set_root(process,fd);
}



SYS_PUBLIC sys_error_t sys_process_get_main_thread(sys_process_t process){
	return _sys_syscall_process_get_main_thread(process);
}
