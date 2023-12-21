#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_linux_execve(syscall_reg_state_t* regs){
	panic("syscall_linux_execve");
}



void syscall_linux_execveat(syscall_reg_state_t* regs){
	panic("syscall_linux_execveat");
}
