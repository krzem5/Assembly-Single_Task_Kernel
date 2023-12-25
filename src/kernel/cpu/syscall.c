#include <kernel/cpu/cpu.h>



u64 syscall_cpu_get_count(void){
	return cpu_count;
}
