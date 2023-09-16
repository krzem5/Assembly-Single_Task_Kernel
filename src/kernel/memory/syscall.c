#include <kernel/config.h>
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
	if (CONFIG_DISABLE_USER_MEMORY_COUNTERS||regs->rsi!=sizeof(pmm_counters_t)||!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	pmm_get_counters((pmm_counters_t*)(regs->rdi));
	regs->rax=1;
}
