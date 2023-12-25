#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



u64 syscall_process_get_pid(void){
	return THREAD_DATA->process->handle.rb_node.key;
}



u64 syscall_thread_get_tid(void){
	return THREAD_DATA->handle.rb_node.key;
}



u64 syscall_process_start(syscall_reg_state_t* regs){
	if (!syscall_get_string_length(regs->rdi)){
		return 0;
	}
	u32 argc=regs->rsi;
	if (argc*sizeof(u64)>syscall_get_user_pointer_max_length(regs->rdx)){
		return 0;
	}
	for (u64 i=0;i<argc;i++){
		if (!syscall_get_string_length(*((u64*)(regs->rdx+i*sizeof(u64))))){
			return 0;
		}
	}
	// copy all vars to a temp buffer + check environ for overflow
	return elf_load((void*)(regs->rdi),argc,(void*)(regs->rdx),(void*)(regs->rcx),regs->r8);
}



u64 syscall_thread_stop(void){
	thread_terminate();
}



u64 syscall_thread_create(syscall_reg_state_t* regs){
	if (!syscall_get_user_pointer_max_length(regs->rdi)){
		return 0;
	}
	u64 stack_size=regs->r8;
	if (!stack_size){
		stack_size=THREAD_DATA->user_stack_region->length;
	}
	thread_t* thread=thread_create_user_thread(THREAD_DATA->process,regs->rdi,stack_size);
	thread->reg_state.gpr_state.rdi=regs->rsi;
	thread->reg_state.gpr_state.rsi=regs->rdx;
	scheduler_enqueue_thread(thread);
	return thread->handle.rb_node.key;
}



u64 syscall_thread_get_priority(syscall_reg_state_t* regs){
	if (!regs->rdi){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,thread_handle_type);
	if (!handle){
		return 0;
	}
	u64 out=((thread_t*)(handle->object))->priority;
	handle_release(handle);
	return out;
}



u64 syscall_thread_set_priority(syscall_reg_state_t* regs){
	if (!regs->rdi||regs->rsi<SCHEDULER_PRIORITY_MIN||regs->rsi>SCHEDULER_PRIORITY_MAX){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,thread_handle_type);
	if (!handle){
		return 0;
	}
	thread_t* thread=handle->object;
	if (thread->state==THREAD_STATE_TYPE_TERMINATED){
		handle_release(handle);
		return 0;
	}
	thread->priority=regs->rsi;
	handle_release(handle);
	return 1;
}



u64 syscall_thread_get_cpu_mask(syscall_reg_state_t* regs){
	u64 size=(regs->rdx>cpu_mask_size?cpu_mask_size:regs->rdx);
	if (!regs->rdi||size>syscall_get_user_pointer_max_length(regs->rsi)){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,thread_handle_type);
	if (!handle){
		return 0;
	}
	thread_t* thread=handle->object;
	memcpy((void*)(regs->rsi),thread->cpu_mask,size);
	handle_release(handle);
	return 1;
}



u64 syscall_thread_set_cpu_mask(syscall_reg_state_t* regs){
	u64 size=(regs->rdx>cpu_mask_size?cpu_mask_size:regs->rdx);
	if (!regs->rdi||size>syscall_get_user_pointer_max_length(regs->rsi)){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,thread_handle_type);
	if (!handle){
		return 0;
	}
	thread_t* thread=handle->object;
	memcpy(thread->cpu_mask,(void*)(regs->rsi),size);
	memset((void*)(((u64)(thread->cpu_mask))+size),0,cpu_mask_size-size);
	handle_release(handle);
	return 1;
}



u64 syscall_thread_await_events(syscall_reg_state_t* regs){
	if (!regs->rsi){
		return -1;
	}
	if (regs->rsi*sizeof(handle_id_t)>syscall_get_user_pointer_max_length(regs->rdi)){
		return -1;
	}
	return event_await_multiple_handles((const handle_id_t*)(regs->rdi),regs->rsi);
}



u64 syscall_process_get_event(syscall_reg_state_t* regs){
	handle_t* handle=handle_lookup_and_acquire(regs->rdi,process_handle_type);
	if (!handle){
		return 0;
	}
	process_t* process=handle->object;
	u64 out=process->event->handle.rb_node.key;
	handle_release(handle);
	return out;
}
