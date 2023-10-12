#include <kernel/aml/runtime.h>
#include <kernel/syscall/syscall.h>



void syscall_aml_get_root_node(syscall_registers_t* regs){
	regs->rax=(u64)aml_root_node;
}
