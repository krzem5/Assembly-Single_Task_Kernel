#include <kernel/acpi/fadt.h>
#include <kernel/context/context.h>
#include <kernel/syscall/syscall.h>



#define USER_SHUTDOWN_FLAG_RESTART 1
#define USER_SHUTDOWN_FLAG_SAVE_CONTEXT 2



void syscall_system_shutdown(syscall_registers_t* regs){
	if (regs->rdi&USER_SHUTDOWN_FLAG_SAVE_CONTEXT){
		context_save();
	}
	acpi_fadt_shutdown(!!(regs->rdi&USER_SHUTDOWN_FLAG_RESTART));
}
