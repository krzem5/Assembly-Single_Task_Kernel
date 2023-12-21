#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_linux_sched_yield(syscall_reg_state_t* regs){
	panic("syscall_linux_sched_yield");
}



void syscall_linux_pause(syscall_reg_state_t* regs){
	panic("syscall_linux_pause");
}



void syscall_linux_nanosleep(syscall_reg_state_t* regs){
	panic("syscall_linux_nanosleep");
}
