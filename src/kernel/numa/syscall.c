#include <kernel/numa/numa.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_numa_node_count(syscall_registers_t* regs){
	regs->rax=numa_node_count;
}



void syscall_numa_node_locality(syscall_registers_t* regs){
	if (regs->rdi>=numa_node_count||regs->rsi>=numa_node_count){
		regs->rax=0;
	}
	else{
		regs->rax=numa_node_locality_matrix[NUMA_LOCALITY_INDEX(regs->rdi,regs->rsi)];
	}
}
