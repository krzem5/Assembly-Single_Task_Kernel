#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/syscall/syscall.h>
#define KERNEL_LOG_NAME "syscall_cpu"



void syscall_cpu_core_count(syscall_registers_t* regs){
	regs->rax=cpu_count;
}



void syscall_cpu_core_start(syscall_registers_t* regs){
	cpu_core_start(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_cpu_core_stop(syscall_registers_t* regs){
	cpu_core_stop();
}
