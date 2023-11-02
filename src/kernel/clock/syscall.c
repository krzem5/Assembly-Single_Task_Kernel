#include <kernel/clock/clock.h>
#include <kernel/isr/isr.h>



void syscall_clock_get_converion(isr_state_t* regs){
	regs->rax=clock_conversion_factor;
	regs->rdx=clock_conversion_shift;
	regs->r8=clock_cpu_frequency;
}
