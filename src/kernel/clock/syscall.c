#include <kernel/clock/clock.h>
#include <kernel/syscall/syscall.h>



u64 syscall_clock_get_converion(syscall_reg_state_t* regs){
	*((u64*)(regs->rdi))=clock_conversion_factor;
	*((u64*)(regs->rdi+8))=clock_conversion_shift;
	*((u64*)(regs->rdi+16))=clock_cpu_frequency;
	return 0;
}
