#include <kernel/cpu/cpu.h>
#include <kernel/syscall/syscall.h>



u64 syscall_cpu_get_count(syscall_reg_state_t* regs){
	return cpu_count;
}
