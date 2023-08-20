#ifndef _KERNEL_NUMA_SYSCALL_H_
#define _KERNEL_NUMA_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_numa_node_count(syscall_registers_t* regs);



#endif
