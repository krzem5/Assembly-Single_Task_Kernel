#include <kernel/gpr/gpr.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "isr"



void _isr_handler(gpr_state_t* gpr_state){
	WARN("ISR %u:",gpr_state->isr);
	WARN("ds     = %p",gpr_state->ds);
	WARN("es     = %p",gpr_state->es);
	WARN("rax    = %p",gpr_state->rax);
	WARN("rbx    = %p",gpr_state->rbx);
	WARN("rcx    = %p",gpr_state->rcx);
	WARN("rdx    = %p",gpr_state->rdx);
	WARN("rsi    = %p",gpr_state->rsi);
	WARN("rdi    = %p",gpr_state->rdi);
	WARN("rbp    = %p",gpr_state->rbp);
	WARN("r8     = %p",gpr_state->r8);
	WARN("r9     = %p",gpr_state->r9);
	WARN("r10    = %p",gpr_state->r10);
	WARN("r11    = %p",gpr_state->r11);
	WARN("r12    = %p",gpr_state->r12);
	WARN("r13    = %p",gpr_state->r13);
	WARN("r14    = %p",gpr_state->r14);
	WARN("r15    = %p",gpr_state->r15);
	WARN("error  = %p",gpr_state->error);
	WARN("rip    = %p",gpr_state->rip);
	WARN("cs     = %p",gpr_state->cs);
	WARN("rflags = %p",gpr_state->rflags);
	WARN("rsp    = %p",gpr_state->rsp);
	WARN("ss     = %p",gpr_state->ss);
	for (;;);
}



void _isr_handler_inside_kernel(void){
	panic("ISR inside kernel",0);
}
