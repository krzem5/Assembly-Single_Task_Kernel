#include <kernel/cpu/cpu.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_cpu_core_start(syscall_registers_t* regs){
	cpu_core_start(regs->rdi,regs->rsi,regs->rdx,regs->r8);
}



void syscall_cpu_core_stop(syscall_registers_t* regs){
	cpu_core_stop();
}
