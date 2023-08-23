#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>




void syscall_memory_map(syscall_registers_t* regs){
	regs->rax=mmap_alloc(regs->rdi,regs->rsi);
}



void syscall_memory_unmap(syscall_registers_t* regs){
	regs->rax=mmap_dealloc(regs->rdi,regs->rsi);
}



void syscall_memory_stats(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(pmm_counters_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	*((pmm_counters_t*)address)=*pmm_get_counters();
	regs->rax=1;
}
