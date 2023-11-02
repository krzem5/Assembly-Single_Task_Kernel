#include <kernel/acpi/fadt.h>
#include <kernel/isr/isr.h>
#include <kernel/types.h>



#define USER_SHUTDOWN_FLAG_RESTART 1



void KERNEL_NORETURN syscall_system_shutdown(isr_state_t* regs){
	acpi_fadt_shutdown(!!(regs->rdi&USER_SHUTDOWN_FLAG_RESTART));
}
