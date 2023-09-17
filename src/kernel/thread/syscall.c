#include <kernel/cpu/cpu.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>



void syscall_thread_stop(syscall_registers_t* regs){
	scheduler_dequeue_thread();
}



void syscall_thread_create(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,1)){
		regs->rax=0;
		return;
	}
	u64 stack_size=regs->r8;
	if (!stack_size){
		stack_size=CPU_HEADER_DATA->cpu_data->scheduler->current_thread->stack_size;
	}
	thread_t* thread=thread_new(CPU_HEADER_DATA->cpu_data->scheduler->current_thread->process,regs->rdi,stack_size);
	thread->gpr_state.rdi=regs->rsi;
	thread->gpr_state.rsi=regs->rdx;
	scheduler_enqueue_thread(thread);
	regs->rax=thread->id;
}
