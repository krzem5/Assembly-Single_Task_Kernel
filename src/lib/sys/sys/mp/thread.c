#include <sys/error/error.h>
#include <sys/mp/event.h>
#include <sys/mp/thread.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



static void _thread_bootstrap(void (*func)(void*),void* arg){
	func(arg);
	_sys_syscall_thread_stop(0);
}



SYS_PUBLIC u64 sys_thread_await_events(const sys_event_t* events,u32 count){
	return _sys_syscall_thread_await_events(events,count);
}



SYS_PUBLIC sys_thread_t sys_thread_create(void* func,void* arg,u64 stack_size){
	return _sys_syscall_thread_create((u64)_thread_bootstrap,(u64)func,(u64)arg,stack_size);
}



SYS_PUBLIC sys_error_t thread_get_cpu_mask(sys_thread_t thread,void* cpumask,u32 cpumask_size){
	return _sys_syscall_thread_get_cpu_mask(thread,cpumask,cpumask_size);
}



SYS_PUBLIC sys_thread_priority_t sys_thread_get_priority(sys_thread_t thread){
	return _sys_syscall_thread_get_priority(thread);
}



SYS_PUBLIC sys_thread_t sys_thread_get_handle(void){
	return _sys_syscall_thread_get_tid();
}



SYS_PUBLIC sys_error_t sys_thread_set_cpu_mask(sys_thread_t thread,const void* cpumask,u32 cpumask_size){
	return _sys_syscall_thread_set_cpu_mask(thread,cpumask,cpumask_size);
}



SYS_PUBLIC sys_error_t sys_thread_set_priority(sys_thread_t thread,sys_thread_priority_t priority){
	return _sys_syscall_thread_set_priority(thread,priority);
}



SYS_PUBLIC sys_error_t sys_thread_stop(sys_thread_t thread){
	return _sys_syscall_thread_stop(thread);
}
