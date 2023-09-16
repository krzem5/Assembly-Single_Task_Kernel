#include <kernel/cpu/cpu.h>
#include <kernel/isr/isr.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "isr"



void _isr_handler(isr_state_t* isr_state){
	if (isr_state->isr==14){
		scheduler_t* scheduler=CPU_DATA->scheduler;
		u64 address=vmm_get_fault_address()&(-PAGE_SIZE);
		if (!(isr_state->error&1)&&scheduler&&scheduler->current_thread&&vmm_virtual_to_physical(&(scheduler->current_thread->process->user_pagemap),address)==VMM_SHADOW_PAGE_ADDRESS){
			vmm_update_address_and_set_present(&(scheduler->current_thread->process->user_pagemap),pmm_alloc_zero(1,PMM_COUNTER_USER,0),address);
			return;
		}
		ERROR("Page Fault");
		ERROR("Address: %p, Error: %p [%u]",vmm_get_fault_address(),isr_state->error,CPU_DATA->index);
	}
	else if (isr_state->isr==32){
		scheduler_isr_handler(isr_state);
		return;
	}
	else if (isr_state->isr>32){
		ERROR("Event interrupt (%u)",isr_state->isr);
		return;
	}
	else{
		ERROR("Crash interrupt");
	}
	WARN("ISR %u:",isr_state->isr);
	WARN("cr3    = %p",isr_state->cr3);
	WARN("es     = %p",isr_state->es);
	WARN("ds     = %p",isr_state->ds);
	WARN("rax    = %p",isr_state->rax);
	WARN("rbx    = %p",isr_state->rbx);
	WARN("rcx    = %p",isr_state->rcx);
	WARN("rdx    = %p",isr_state->rdx);
	WARN("rsi    = %p",isr_state->rsi);
	WARN("rdi    = %p",isr_state->rdi);
	WARN("rbp    = %p",isr_state->rbp);
	WARN("r8     = %p",isr_state->r8);
	WARN("r9     = %p",isr_state->r9);
	WARN("r10    = %p",isr_state->r10);
	WARN("r11    = %p",isr_state->r11);
	WARN("r12    = %p",isr_state->r12);
	WARN("r13    = %p",isr_state->r13);
	WARN("r14    = %p",isr_state->r14);
	WARN("r15    = %p",isr_state->r15);
	WARN("error  = %p",isr_state->error);
	WARN("rip    = %p",isr_state->rip);
	WARN("cs     = %p",isr_state->cs);
	WARN("rflags = %p",isr_state->rflags);
	WARN("rsp    = %p",isr_state->rsp);
	WARN("ss     = %p",isr_state->ss);
	if (isr_state->cs==8){
		u64 rbp=isr_state->rbp;
		while (rbp){
			LOG("[%u] %p ~ %p",CPU_DATA->index,rbp);
			LOG("[%u] %p",CPU_DATA->index,*((u64*)(rbp+8)));
			rbp=*((u64*)rbp);
		}
	}
	for (;;);
}
