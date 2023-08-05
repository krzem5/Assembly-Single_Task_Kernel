#include <kernel/context/context.h>
#include <kernel/syscall/syscall.h>



void syscall_context_save(syscall_registers_t* regs){
	context_save();
}
