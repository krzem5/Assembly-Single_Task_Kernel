#include <kernel/clock/clock.h>
#include <kernel/syscall/syscall.h>



u64 syscall_clock_get_converion(syscall_reg_state_t* regs){
	regs->rdx=clock_conversion_shift;
	regs->r8=clock_cpu_frequency;
	return clock_conversion_factor;
}
