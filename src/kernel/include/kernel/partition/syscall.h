#ifndef _KERNEL_PARTITION_SYSCALL_H_
#define _KERNEL_PARTITION_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_file_system_count(syscall_registers_t* regs);



void syscall_file_system_get(syscall_registers_t* regs);



#endif
