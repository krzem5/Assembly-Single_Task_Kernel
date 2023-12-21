#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_linux_fork(syscall_reg_state_t* regs){
	panic("syscall_linux_fork");
}



void syscall_linux_vfork(syscall_reg_state_t* regs){
	panic("syscall_linux_vfork");
}
