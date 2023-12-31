#include <sys/_kernel_syscall.h>
#include <sys/types.h>



SYS_PUBLIC u32 sys_cpu_get_count(void){
	return _syscall_cpu_get_count();
}
