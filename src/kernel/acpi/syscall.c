#include <kernel/shutdown/shutdown.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



#define USER_SHUTDOWN_FLAG_RESTART 1



void KERNEL_NORETURN syscall_system_shutdown(syscall_reg_state_t* regs){
	shutdown(!!(regs->rdi&USER_SHUTDOWN_FLAG_RESTART));
}
