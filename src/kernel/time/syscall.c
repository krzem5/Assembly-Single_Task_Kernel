#include <kernel/syscall/syscall.h>
#include <kernel/time/time.h>



void syscall_time_get_boot_offset(syscall_reg_state_t* regs){
	regs->rax=time_boot_offset;
}
