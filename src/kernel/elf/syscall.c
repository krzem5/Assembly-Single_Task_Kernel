#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_elf_load(syscall_registers_t* regs){
	regs->rax=-1;
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		return;
	}
	char buffer[4096];
	if (regs->rsi>4095){
		return;
	}
	memcpy(buffer,VMM_TRANSLATE_ADDRESS(address),regs->rsi);
	buffer[regs->rsi]=0;
	u64 start_address=elf_load(buffer);
	if (!start_address){
		return;
	}
	cpu_core_start(cpu_bsp_core_id,start_address,0);
}
