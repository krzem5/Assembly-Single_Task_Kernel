#include <kernel/cpu/cpu.h>
#include <kernel/syscall/syscall.h>



void syscall_cpu_get_count(syscall_registers_t* regs){
	regs->rax=cpu_count;
}
