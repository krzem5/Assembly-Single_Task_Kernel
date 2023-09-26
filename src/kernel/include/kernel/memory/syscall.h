#ifndef _KERNEL_MEMORY_SYSCALL_H_
#define _KERNEL_MEMORY_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_memory_map(syscall_registers_t* regs);



void syscall_memory_unmap(syscall_registers_t* regs);



void syscall_memory_get_counter_count(syscall_registers_t* regs);



void syscall_memory_get_counter(syscall_registers_t* regs);



void syscall_memory_get_object_counter_count(syscall_registers_t* regs);



void syscall_memory_get_object_counter(syscall_registers_t* regs);



#endif
