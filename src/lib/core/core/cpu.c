#include <core/syscall.h>
#include <core/types.h>



CORE_PUBLIC u32 cpu_get_count(void){
	return _syscall_cpu_get_count();
}
