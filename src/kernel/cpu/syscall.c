#include <kernel/cpu/cpu.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_cpu_core_count(syscall_registers_t* regs){
	regs->rax=cpu_count|(((u64)cpu_bsp_core_id)<<32);
}



void syscall_cpu_core_start(syscall_registers_t* regs){
	cpu_core_start(regs->rdi,regs->rsi,regs->rdx);
}



void KERNEL_NORETURN syscall_cpu_core_stop(syscall_registers_t* regs){
	cpu_core_stop();
}
