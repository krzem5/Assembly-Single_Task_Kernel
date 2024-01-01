#include <sys2/syscall/kernel_syscalls.h>



SYS2_PUBLIC void sys2_scheduler_yield(void){
	_sys2_syscall_scheduler_yield();
}
