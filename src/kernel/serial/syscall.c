#include <kernel/memory/vmm.h>
#include <kernel/serial/serial.h>
#include <kernel/syscall/syscall.h>



void syscall_serial_send(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		return;
	}
	serial_send((const void*)(regs->rdi),regs->rsi);
}



void syscall_serial_recv(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	regs->rax=serial_recv((void*)(regs->rdi),regs->rsi,regs->rdx);
}
