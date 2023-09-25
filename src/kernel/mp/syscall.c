#include <kernel/cpu/cpu.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/mp/thread.h>
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



void syscall_thread_get_priority(syscall_registers_t* regs){
	if (!regs->rdi){
		regs->rax=0;
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_THREAD);
	if (!handle){
		regs->rax=0;
		return;
	}
	regs->rax=((thread_t*)(handle->object))->priority;
	handle_release(handle);
}



void syscall_thread_set_priority(syscall_registers_t* regs){
	if (!regs->rdi||regs->rsi<SCHEDULER_PRIORITY_MIN||regs->rsi>SCHEDULER_PRIORITY_MAX){
		regs->rax=0;
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_THREAD);
	if (!handle){
		regs->rax=0;
		return;
	}
	thread_t* thread=handle->object;
	if (thread->state.type==THREAD_STATE_TYPE_TERMINATED){
		regs->rax=0;
		handle_release(handle);
		return;
	}
	thread->priority=regs->rsi;
	handle_release(handle);
	regs->rax=1;
}
