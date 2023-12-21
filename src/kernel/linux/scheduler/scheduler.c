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



void syscall_linux_getpriority(syscall_reg_state_t* regs){
	panic("syscall_linux_getpriority");
}



void syscall_linux_setpriority(syscall_reg_state_t* regs){
	panic("syscall_linux_setpriority");
}



void syscall_linux_sched_get_priority_max(syscall_reg_state_t* regs){
	panic("syscall_linux_sched_get_priority_max");
}



void syscall_linux_sched_get_priority_min(syscall_reg_state_t* regs){
	panic("syscall_linux_sched_get_priority_min");
}



void syscall_linux_sched_rr_get_interval(syscall_reg_state_t* regs){
	panic("syscall_linux_sched_rr_get_interval");
}



void syscall_linux_sched_setaffinity(syscall_reg_state_t* regs){
	panic("syscall_linux_sched_setaffinity");
}



void syscall_linux_sched_getaffinity(syscall_reg_state_t* regs){
	panic("syscall_linux_sched_getaffinity");
}
