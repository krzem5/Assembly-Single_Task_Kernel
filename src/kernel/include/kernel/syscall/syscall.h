#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/types.h>



void syscall_invalid(isr_state_t* regs);



_Bool syscall_sanatize_user_memory(u64 address,u64 size);



void syscall_enable(void);



#endif
