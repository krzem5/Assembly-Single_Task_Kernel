#include <sys/mp/process.h>
#include <sys/mp/process_group.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_process_group_t sys_process_group_get(sys_process_t process){
	return _sys_syscall_process_group_get(process);
}



SYS_PUBLIC sys_process_group_t sys_process_group_set(sys_process_t process,sys_process_group_t process_group){
	return _sys_syscall_process_group_set(process,process_group);
}



SYS_PUBLIC sys_process_group_t sys_process_group_iter_start(void){
	return _sys_syscall_process_group_get_next(0);
}



SYS_PUBLIC sys_process_group_t sys_process_group_iter_next(sys_process_group_t process_group){
	return _sys_syscall_process_group_get_next(process_group);
}



SYS_PUBLIC sys_process_t sys_process_group_iter_group_start(sys_process_group_t process_group){
	return _sys_syscall_process_group_iter(process_group,0);
}



SYS_PUBLIC sys_process_t sys_process_group_iter_group_next(sys_process_group_t process_group,sys_process_t process){
	return _sys_syscall_process_group_iter(process_group,process);
}
