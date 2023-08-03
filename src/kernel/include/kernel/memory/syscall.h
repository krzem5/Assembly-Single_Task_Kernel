#ifndef _KERNEL_MEMORY_SYSCALL_H_
#define _KERNEL_MEMORY_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_memory_map(syscall_registers_t* regs);



void syscall_memory_unmap(syscall_registers_t* regs);



void syscall_memory_stats(syscall_registers_t* regs);



#endif
