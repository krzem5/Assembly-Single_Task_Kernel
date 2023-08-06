#include <kernel/bios/bios.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_system_config(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(bios_data_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	*((bios_data_t*)VMM_TRANSLATE_ADDRESS(address))=bios_data;
	regs->rax=1;
}
