#include <kernel/isr/isr.h>
#include <kernel/memory/vmm.h>
#include <kernel/random/random.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_random_generate(isr_state_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	random_generate((void*)(regs->rdi),regs->rsi);
	regs->rax=1;
}
