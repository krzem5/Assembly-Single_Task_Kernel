#ifndef _KERNEL_NETWORK_SYSCALL_H_
#define _KERNEL_NETWORK_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_network_send(syscall_registers_t* regs);



void syscall_network_poll(syscall_registers_t* regs);



void syscall_network_config(syscall_registers_t* regs);



#endif
