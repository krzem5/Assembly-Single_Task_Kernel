#ifndef _KERNEL_HANDLE_SYSCALL_H_
#define _KERNEL_HANDLE_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_handle_get_type_count(syscall_registers_t* regs);



void syscall_handle_get_type(syscall_registers_t* regs);



#endif
