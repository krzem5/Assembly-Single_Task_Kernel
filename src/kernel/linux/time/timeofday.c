#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_linux_gettimeofday(syscall_reg_state_t* regs){
	panic("syscall_linux_gettimeofday");
}



void syscall_linux_settimeofday(syscall_reg_state_t* regs){
	panic("syscall_linux_settimeofday");
}
