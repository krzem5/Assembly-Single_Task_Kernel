#include <user/syscall.h>
#include <user/types.h>
#include <user/io.h>



u32 cpu_count;



void cpu_init(void){
	cpu_count=_syscall_cpu_core_count();
}
