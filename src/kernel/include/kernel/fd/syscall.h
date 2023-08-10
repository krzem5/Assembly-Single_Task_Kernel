#ifndef _KERNEL_FD_SYSCALL_H_
#define _KERNEL_FD_SYSCALL_H_ 1
#include <kernel/syscall/syscall.h>



void syscall_fd_open(syscall_registers_t* regs);



void syscall_fd_close(syscall_registers_t* regs);



void syscall_fd_delete(syscall_registers_t* regs);



void syscall_fd_read(syscall_registers_t* regs);



void syscall_fd_write(syscall_registers_t* regs);



void syscall_fd_seek(syscall_registers_t* regs);



void syscall_fd_resize(syscall_registers_t* regs);



void syscall_fd_absolute_path(syscall_registers_t* regs);



void syscall_fd_stat(syscall_registers_t* regs);



void syscall_fd_get_relative(syscall_registers_t* regs);



void syscall_fd_move(syscall_registers_t* regs);



#endif
