#ifndef _KERNEL_RANDOM_SYSCALL_H_
#define _KERNEL_RANDOM_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_random_generate(syscall_registers_t* regs);



#endif
