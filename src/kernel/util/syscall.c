#include <kernel/syscall/syscall.h>
#include <kernel/user/data.h>



void syscall_user_data_pointer(syscall_registers_t* regs){
	regs->rax=(u64)user_data_pointer;
}
