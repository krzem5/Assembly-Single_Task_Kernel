#include <sys/_kernel_syscall.h>
#include <sys/thread.h>
#include <sys/types.h>



static void _sys_thread_bootstrap(void (*func)(void*),void* arg){
	func(arg);
	sys_thread_stop();
}



SYS_PUBLIC void __attribute__((noreturn)) sys_thread_stop(void){
	_syscall_thread_stop(0);
	for (;;);
}



SYS_PUBLIC u64 sys_thread_create(void (*func)(void*),void* arg,u64 stack_size){
	return _syscall_thread_create((u64)_sys_thread_bootstrap,(u64)func,(u64)arg,stack_size);
}



SYS_PUBLIC u32 sys_thread_get_priority(u64 handle){
	return _syscall_thread_get_priority(handle);
}



SYS_PUBLIC _Bool sys_thread_set_priority(u64 handle,u32 priority){
	return _syscall_thread_set_priority(handle,priority);
}
