#include <kernel/config.h>
#include <kernel/cpu/cpu.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>




void syscall_memory_map(syscall_registers_t* regs){
	u64 length=pmm_align_up_address(regs->rdi);
	u64 out=vmm_memory_map_reserve(&(CPU_HEADER_DATA->cpu_data->scheduler->current_thread->process->mmap),0,length);
	if (out){
		vmm_reserve_pages(&(CPU_HEADER_DATA->cpu_data->scheduler->current_thread->process->user_pagemap),out,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE,length>>PAGE_SIZE_SHIFT);
	}
	regs->rax=out;
}



void syscall_memory_unmap(syscall_registers_t* regs){
	u64 length=pmm_align_up_address(regs->rsi);
	if (!vmm_memory_map_release(&(CPU_HEADER_DATA->cpu_data->scheduler->current_thread->process->mmap),regs->rdi,length)){
		regs->rax=0;
		return;
	}
	vmm_release_pages(&(CPU_HEADER_DATA->cpu_data->scheduler->current_thread->process->user_pagemap),regs->rdi,length>>PAGE_SIZE_SHIFT);
	regs->rax=1;
}



void syscall_memory_stats(syscall_registers_t* regs){
	if (CONFIG_DISABLE_USER_MEMORY_COUNTERS||regs->rsi!=sizeof(pmm_counters_t)||!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	pmm_get_counters((pmm_counters_t*)(regs->rdi));
	regs->rax=1;
}
