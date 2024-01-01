#include <sys/syscall/kernel_syscalls.h>



SYS_PUBLIC void sys_scheduler_yield(void){
	_sys_syscall_scheduler_yield();
}
