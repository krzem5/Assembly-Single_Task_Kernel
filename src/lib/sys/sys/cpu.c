#include <sys/syscall.h>
#include <sys/types.h>



SYS_PUBLIC u32 cpu_get_count(void){
	return _syscall_cpu_get_count();
}
