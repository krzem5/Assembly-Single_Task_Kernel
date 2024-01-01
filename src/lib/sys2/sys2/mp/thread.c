#include <sys2/error/error.h>
#include <sys2/mp/event.h>
#include <sys2/mp/thread.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



static void _thread_bootstrap(void (*func)(void*),void* arg){
	func(arg);
	_sys2_syscall_thread_stop(0);
}



SYS2_PUBLIC u64 sys2_thread_await_events(const sys2_event_t* events,u32 count){
	return _sys2_syscall_thread_await_events(events,count);
}



SYS2_PUBLIC sys2_thread_t sys2_thread_create(void* func,void* arg,u64 stack_size){
	return _sys2_syscall_thread_create((u64)_thread_bootstrap,(u64)func,(u64)arg,stack_size);
}



SYS2_PUBLIC sys2_error_t thread_get_cpu_mask(sys2_thread_t thread,void* cpumask,u32 cpumask_size){
	return _sys2_syscall_thread_get_cpu_mask(thread,cpumask,cpumask_size);
}



SYS2_PUBLIC sys2_thread_priority_t sys2_thread_get_priority(sys2_thread_t thread){
	return _sys2_syscall_thread_get_priority(thread);
}



SYS2_PUBLIC sys2_thread_t sys2_thread_get_handle(void){
	return _sys2_syscall_thread_get_tid();
}



SYS2_PUBLIC sys2_error_t sys2_thread_set_cpu_mask(sys2_thread_t thread,void* cpumask,u32 cpumask_size){
	return _sys2_syscall_thread_set_cpu_mask(thread,cpumask,cpumask_size);
}



SYS2_PUBLIC sys2_error_t sys2_thread_set_priority(sys2_thread_t thread,sys2_thread_priority_t priority){
	return _sys2_syscall_thread_set_priority(thread,priority);
}



SYS2_PUBLIC sys2_error_t sys2_thread_stop(sys2_thread_t thread){
	return _sys2_syscall_thread_stop(thread);
}
