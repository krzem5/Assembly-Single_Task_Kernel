#ifndef _KERNEL_COVERAGE_SYSCALL_H_
#define _KERNEL_COVERAGE_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_coverage_dump_data(syscall_registers_t* regs);



#endif
