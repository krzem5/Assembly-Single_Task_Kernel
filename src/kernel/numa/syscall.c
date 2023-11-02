#include <kernel/isr/isr.h>
#include <kernel/numa/numa.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



typedef struct _USER_NUMA_NODE{
	u32 index;
	u32 cpu_count;
	u32 memory_range_count;
} user_numa_node_t;



typedef struct _USER_NUMA_CPU{
	u32 numa_index;
	u32 cpu_index;
	u8 apic_id;
	u32 sapic_eid;
} user_numa_cpu_t;



typedef struct _USER_NUMA_MEMORY_RANGE{
	u32 numa_index;
	u32 memory_range_index;
	u64 base_address;
	u64 length;
	_Bool hot_pluggable;
} user_numa_memory_range_t;



void syscall_numa_get_node_count(isr_state_t* regs){
	regs->rax=numa_node_count;
}



void syscall_numa_get_node(isr_state_t* regs){
	if (regs->rdi>=numa_node_count||regs->rdx!=sizeof(user_numa_node_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const numa_node_t* node=numa_nodes+regs->rdi;
	user_numa_node_t* out=(user_numa_node_t*)(regs->rsi);
	out->index=node->index;
	out->cpu_count=node->cpu_count;
	out->memory_range_count=node->memory_range_count;
	regs->rax=1;
}



void syscall_numa_get_node_cpu(isr_state_t* regs){
	if (regs->rdi>=numa_node_count||regs->r8!=sizeof(user_numa_cpu_t)||!syscall_sanatize_user_memory(regs->rdx,regs->r8)){
		regs->rax=0;
		return;
	}
	const numa_node_t* node=numa_nodes+regs->rdi;
	if (regs->rsi>=node->cpu_count){
		regs->rax=0;
		return;
	}
	const numa_cpu_t* cpu=node->cpus;
	for (u32 i=0;i<regs->rsi;i++){
		cpu=cpu->next;
	}
	user_numa_cpu_t* out=(user_numa_cpu_t*)(regs->rdx);
	out->numa_index=regs->rdi;
	out->cpu_index=regs->rsi;
	out->apic_id=cpu->apic_id;
	out->sapic_eid=cpu->sapic_eid;
	regs->rax=1;
}



void syscall_numa_get_node_memory_range(isr_state_t* regs){
	if (regs->rdi>=numa_node_count||regs->r8!=sizeof(user_numa_memory_range_t)||!syscall_sanatize_user_memory(regs->rdx,regs->r8)){
		regs->rax=0;
		return;
	}
	const numa_node_t* node=numa_nodes+regs->rdi;
	if (regs->rsi>=node->memory_range_count){
		regs->rax=0;
		return;
	}
	const numa_memory_range_t* memory_range=node->memory_ranges;
	for (u32 i=0;i<regs->rsi;i++){
		memory_range=memory_range->next;
	}
	user_numa_memory_range_t* out=(user_numa_memory_range_t*)(regs->rdx);
	out->numa_index=regs->rdi;
	out->memory_range_index=regs->rsi;
	out->base_address=memory_range->base_address;
	out->length=memory_range->length;
	out->hot_pluggable=memory_range->hot_pluggable;
	regs->rax=1;
}



void syscall_numa_get_locality(isr_state_t* regs){
	u64 matrix_size=numa_node_count*numa_node_count;
	if (regs->rdi>=matrix_size||regs->rdx>=matrix_size||regs->rdi+regs->rdx>=matrix_size||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	memcpy((void*)(regs->rsi),numa_node_locality_matrix+regs->rdi,regs->rdx);
	regs->rax=1;
}
