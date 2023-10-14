#include <user/syscall.h>
#include <user/types.h>



u32 cpu_get_count(void){
	return _syscall_cpu_get_count();
}
