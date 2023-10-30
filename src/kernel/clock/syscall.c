#include <kernel/clock/clock.h>
#include <kernel/syscall/syscall.h>



void syscall_cspinlock_get_converion(syscall_registers_t* regs){
	regs->rax=cspinlock_conversion_factor;
	regs->rdx=cspinlock_conversion_shift;
	regs->r8=cspinlock_cpu_frequency;
}
