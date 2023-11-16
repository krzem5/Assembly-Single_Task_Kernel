#include <kernel/isr/isr.h>
#include <kernel/time/time.h>



void syscall_time_get_boot_offset(isr_state_t* regs){
	regs->rax=time_boot_offset;
}
