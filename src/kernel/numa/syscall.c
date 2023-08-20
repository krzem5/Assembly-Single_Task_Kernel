#include <kernel/numa/numa.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_numa_node_count(syscall_registers_t* regs){
	regs->rax=numa_node_count;
}
