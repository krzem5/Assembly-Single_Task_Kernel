#ifndef _KERNEL_THREAD_SYSCALL_H_
#define _KERNEL_THREAD_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_thread_stop(syscall_registers_t* regs);



void syscall_thread_create(syscall_registers_t* regs);



void syscall_thread_get_priority(syscall_registers_t* regs);



void syscall_thread_set_priority(syscall_registers_t* regs);



void syscall_thread_get_cpu_mask(syscall_registers_t* regs);



void syscall_thread_set_cpu_mask(syscall_registers_t* regs);



#endif
