#ifndef _KERNEL_SYSCALL_SYSCALL_H_
#define _KERNEL_SYSCALL_SYSCALL_H_ 1
#include <kernel/types.h>



#define SYSCALL_COUNT 23



void syscall_init(void);



void syscall_enable(void);



void syscall_jump_to_user_mode(u64 fn,u64 arg,u64 stack_top);



#endif
