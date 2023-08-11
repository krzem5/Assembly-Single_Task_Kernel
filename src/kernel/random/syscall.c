#include <kernel/memory/vmm.h>
#include <kernel/random/random.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_random_generate(syscall_registers_t* regs){
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	random_generate(VMM_TRANSLATE_ADDRESS(address),regs->rsi);
	regs->rax=1;
}
