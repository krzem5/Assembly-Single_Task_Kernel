#include <kernel/cpu/cpu.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>



void syscall_thread_stop(syscall_registers_t* regs){
	thread_terminate();
}



void syscall_thread_create(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,1)){
		regs->rax=0;
		return;
	}
	u64 stack_size=regs->r8;
	if (!stack_size){
		stack_size=THREAD_DATA->stack_size;
	}
	thread_t* thread=thread_new(THREAD_DATA->process,regs->rdi,stack_size);
	thread->gpr_state.rdi=regs->rsi;
	thread->gpr_state.rsi=regs->rdx;
	scheduler_enqueue_thread(thread);
	regs->rax=thread->handle.id;
}
