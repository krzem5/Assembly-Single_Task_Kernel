#include <kernel/memory/vmm.h>
#include <kernel/serial/serial.h>
#include <kernel/syscall/syscall.h>



void syscall_serial_send(syscall_registers_t* regs){
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		return;
	}
	serial_send((void*)address,regs->rsi);
}



void syscall_serial_recv(syscall_registers_t* regs){
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	regs->rax=serial_recv((void*)address,regs->rsi,regs->rdx);
}
