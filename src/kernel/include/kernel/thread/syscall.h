#ifndef _KERNEL_THREAD_SYSCALL_H_
#define _KERNEL_THREAD_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_thread_stop(syscall_registers_t* regs);



void syscall_thread_create(syscall_registers_t* regs);



#endif
