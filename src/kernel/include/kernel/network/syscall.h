#ifndef _KERNEL_NETWORK_SYSCALL_H_
#define _KERNEL_NETWORK_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_net_send(syscall_registers_t* regs);



void syscall_net_poll(syscall_registers_t* regs);



#endif
