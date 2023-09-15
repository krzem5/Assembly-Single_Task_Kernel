#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_elf_load(syscall_registers_t* regs){
	regs->rax=0;
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		return;
	}
	char buffer[4096];
	if (regs->rsi>4095){
		return;
	}
	memcpy(buffer,(void*)address,regs->rsi);
	buffer[regs->rsi]=0;
	regs->rax=elf_load(buffer);
}
