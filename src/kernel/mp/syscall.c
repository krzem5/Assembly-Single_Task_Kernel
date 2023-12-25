#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/handle/handle.h>
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



u64 syscall_process_start(const char* path,u32 argc,const char*const* argv,const char*const* environ,u32 flags){
	if (!syscall_get_string_length((u64)path)){
		return 0;
	}
	if (argc*sizeof(u64)>syscall_get_user_pointer_max_length((u64)argv)){
		return 0;
	}
	for (u64 i=0;i<argc;i++){
		if (!syscall_get_string_length((u64)(argv[i]))){
			return 0;
		}
	}
	// copy all vars to a temp buffer + check environ for overflow
	return elf_load(path,argc,argv,environ,flags);
}



u64 syscall_thread_stop(void){
	thread_terminate();
}



u64 syscall_thread_create(u64 func,u64 arg0,u64 arg1,u64 stack_size){
	if (!syscall_get_user_pointer_max_length(func)){
		return 0;
	}
	thread_t* thread=thread_create_user_thread(THREAD_DATA->process,func,(stack_size?stack_size:THREAD_DATA->user_stack_region->length));
	thread->reg_state.gpr_state.rdi=arg0;
	thread->reg_state.gpr_state.rsi=arg1;
	scheduler_enqueue_thread(thread);
	return thread->handle.rb_node.key;
}



u64 syscall_thread_get_priority(handle_id_t thread_handle){
	if (!thread_handle){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return 0;
	}
	u64 out=((thread_t*)(handle->object))->priority;
	handle_release(handle);
	return out;
}



u64 syscall_thread_set_priority(handle_id_t thread_handle,u64 priority){
	if (!thread_handle||priority<SCHEDULER_PRIORITY_MIN||priority>SCHEDULER_PRIORITY_MAX){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return 0;
	}
	thread_t* thread=handle->object;
	if (thread->state==THREAD_STATE_TYPE_TERMINATED){
		handle_release(handle);
		return 0;
	}
	thread->priority=priority;
	handle_release(handle);
	return 1;
}



u64 syscall_thread_get_cpu_mask(handle_id_t thread_handle,void* buffer,u32 buffer_size){
	if (buffer_size>cpu_mask_size){
		buffer_size=cpu_mask_size;
	}
	if (!thread_handle||buffer_size>syscall_get_user_pointer_max_length((u64)buffer)){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return 0;
	}
	thread_t* thread=handle->object;
	memcpy(buffer,thread->cpu_mask,buffer_size);
	handle_release(handle);
	return 1;
}



u64 syscall_thread_set_cpu_mask(handle_id_t thread_handle,const void* buffer,u32 buffer_size){
	if (buffer_size>cpu_mask_size){
		buffer_size=cpu_mask_size;
	}
	if (!thread_handle||buffer_size>syscall_get_user_pointer_max_length((u64)buffer)){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return 0;
	}
	thread_t* thread=handle->object;
	memcpy(thread->cpu_mask,buffer,buffer_size);
	memset((void*)(((u64)(thread->cpu_mask))+buffer_size),0,cpu_mask_size-buffer_size);
	handle_release(handle);
	return 1;
}



u64 syscall_thread_await_events(const handle_id_t* events,u64 event_count){
	if (!event_count){
		return -1;
	}
	if (event_count*sizeof(handle_id_t)>syscall_get_user_pointer_max_length((u64)events)){
		return -1;
	}
	return event_await_multiple_handles(events,event_count);
}



u64 syscall_process_get_event(handle_id_t process_handle){
	if (!process_handle){
		return 0;
	}
	handle_t* handle=handle_lookup_and_acquire(process_handle,process_handle_type);
	if (!handle){
		return 0;
	}
	process_t* process=handle->object;
	u64 out=process->event->handle.rb_node.key;
	handle_release(handle);
	return out;
}
