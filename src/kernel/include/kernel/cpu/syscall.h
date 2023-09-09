#ifndef _KERNEL_CPU_SYSCALL_H_
#define _KERNEL_CPU_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_cpu_core_start(syscall_registers_t* regs);



void syscall_cpu_core_stop(syscall_registers_t* regs);



#endif
