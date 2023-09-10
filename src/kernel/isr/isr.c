#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "isr"



typedef struct _GPR_STATE{
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 isr;
	u64 error;
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} gpr_state_t;



void _isr_handler(gpr_state_t* gpr_state){
	WARN("ISR %u:",gpr_state->isr);
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
	ERROR("ISR inside kernel");
	for (;;);
}
