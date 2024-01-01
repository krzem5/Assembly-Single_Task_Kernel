#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC u32 sys_cpu_get_count(void){
	return _sys_syscall_cpu_get_count();
}
