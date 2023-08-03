#ifndef _KERNEL_DRIVE_SYSCALL_H_
#define _KERNEL_DRIVE_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_drive_format(syscall_registers_t* regs);



void syscall_drive_stats(syscall_registers_t* regs);



#endif
