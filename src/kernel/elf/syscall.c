#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_elf_load(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	char buffer[4096];
	if (regs->rsi>4095){
		regs->rax=0;
		return;
	}
	memcpy(buffer,(void*)(regs->rdi),regs->rsi);
	buffer[regs->rsi]=0;
	regs->rax=elf_load(buffer);
}
