#include <kernel/cpu/cpu.h>
#include <kernel/isr/isr.h>



void syscall_cpu_get_count(isr_state_t* regs){
	regs->rax=cpu_count;
}
