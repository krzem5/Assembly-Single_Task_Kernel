#include <kernel/cpu/cpu.h>
#include <kernel/isr/isr.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_thread_stop(isr_state_t* regs){
	thread_terminate();
}



void syscall_thread_create(isr_state_t* regs){
	if (!syscall_sanatize_user_memory(regs->rdi,1)){
		regs->rax=0;
		return;
	}
	u64 stack_size=regs->r8;
	if (!stack_size){
		stack_size=THREAD_DATA->user_stack_size;
	}
	thread_t* thread=thread_new_user_thread(THREAD_DATA->process,regs->rdi,stack_size);
	thread->gpr_state.rdi=regs->rsi;
	thread->gpr_state.rsi=regs->rdx;
	scheduler_enqueue_thread(thread);
	regs->rax=thread->handle.rb_node.key;
}



void syscall_thread_get_priority(isr_state_t* regs){
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



void syscall_thread_set_priority(isr_state_t* regs){
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



void syscall_thread_get_cpu_mask(isr_state_t* regs){
	u64 size=(regs->rdx>cpu_mask_size?cpu_mask_size:regs->rdx);
	if (!regs->rdi||!syscall_sanatize_user_memory(regs->rsi,size)){
		regs->rax=0;
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_THREAD);
	if (!handle){
		regs->rax=0;
		return;
	}
	thread_t* thread=handle->object;
	memcpy((void*)(regs->rsi),thread->cpu_mask,size);
	handle_release(handle);
	regs->rax=1;
}



void syscall_thread_set_cpu_mask(isr_state_t* regs){
	u64 size=(regs->rdx>cpu_mask_size?cpu_mask_size:regs->rdx);
	if (!regs->rdi||!syscall_sanatize_user_memory(regs->rsi,size)){
		regs->rax=0;
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,HANDLE_TYPE_THREAD);
	if (!handle){
		regs->rax=0;
		return;
	}
	thread_t* thread=handle->object;
	memcpy(thread->cpu_mask,(void*)(regs->rsi),size);
	memset((void*)(((u64)(thread->cpu_mask))+size),0,cpu_mask_size-size);
	handle_release(handle);
	regs->rax=1;
}
