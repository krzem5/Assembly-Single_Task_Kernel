#ifndef _KERNEL_SERIAL_SYCALL_H_
#define _KERNEL_SERIAL_SYCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_serial_send(syscall_registers_t* regs);



void syscall_serial_recv(syscall_registers_t* regs);



#endif
