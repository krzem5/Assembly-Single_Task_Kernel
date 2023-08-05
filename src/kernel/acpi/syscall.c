#include <kernel/acpi/fadt.h>
#include <kernel/syscall/syscall.h>



#define USER_SHUTDOWN_FLAG_RESTART 1



void syscall_system_shutdown(syscall_registers_t* regs){
	acpi_fadt_shutdown(!!(regs->rdi&USER_SHUTDOWN_FLAG_RESTART));
}
