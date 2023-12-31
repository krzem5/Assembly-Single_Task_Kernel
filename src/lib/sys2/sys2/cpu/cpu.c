#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC u32 sys2_cpu_get_count(void){
	return _sys2_syscall_cpu_get_count();
}
