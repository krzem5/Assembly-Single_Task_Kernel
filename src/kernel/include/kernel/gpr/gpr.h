#ifndef _KERNEL_GPR_GPR_H_
#define _KERNEL_GPR_GPR_H_ 1
#include <kernel/types.h>



typedef struct _GPR_STATE{
	u64 cr3;
	u64 es;
	u64 ds;
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



#endif
