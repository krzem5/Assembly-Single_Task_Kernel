#include <kernel/acpi/fadt.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



#define USER_SHUTDOWN_FLAG_RESTART 1



void KERNEL_NORETURN syscall_system_shutdown(syscall_registers_t* regs){
	acpi_fadt_shutdown(!!(regs->rdi&USER_SHUTDOWN_FLAG_RESTART));
}
