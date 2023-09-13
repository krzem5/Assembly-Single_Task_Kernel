#include <kernel/isr/isr.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "isr"



void _isr_handler(isr_state_t* isr_state){
	if (isr_state->isr==14){
		ERROR("Page Fault");
		ERROR("Address: %p, Error: %p",vmm_get_fault_address(),isr_state->error);
	}
	else if (isr_state->isr==32){
		ERROR("Scheduler");
	}
	else if (isr_state->isr>32){
		ERROR("Event interrupt");
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
	for (;;);
}



void _isr_handler_inside_kernel(void){
	panic("ISR inside kernel",0);
}
