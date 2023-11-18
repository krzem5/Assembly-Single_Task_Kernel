#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/isr/isr.h>
#include <kernel/types.h>



void syscall_invalid(isr_state_t* regs);



u64 syscall_get_user_pointer_max_length(u64 address);



u64 syscall_get_string_length(u64 address);



void syscall_enable(void);



#endif
