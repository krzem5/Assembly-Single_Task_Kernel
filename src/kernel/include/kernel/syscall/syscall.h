#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/types.h>



typedef struct _SYSCALL_REGISTERS{
	u64 rax;
	u64 rbx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 rflags;
	u64 rip;
} syscall_registers_t;



void syscall_init(void);



u64 syscall_sanatize_user_memory(u64 start,u64 size);



void syscall_enable(void);



void syscall_jump_to_user_mode(u64 fn,u64 arg,u64 stack_top);



#endif
