#ifndef _KERNEL_ACPI_SYSCALL_H_
#define _KERNEL_ACPI_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void KERNEL_NORETURN syscall_system_shutdown(syscall_registers_t* regs);



#endif
