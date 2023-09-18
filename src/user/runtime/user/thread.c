#include <user/syscall.h>
#include <user/thread.h>
#include <user/types.h>



static void _thread_bootstrap(void (*func)(void*),void* arg){
	func(arg);
	thread_stop();
}



void __attribute__((noreturn)) thread_stop(void){
	_syscall_thread_stop();
}



u64 thread_create(void (*func)(void*),void* arg,u64 stack_size){
	return _syscall_thread_create((u64)_thread_bootstrap,(u64)func,(u64)arg,stack_size);
}



_Bool thread_set_priority(u64 handle,u32 priority){
	return _syscall_thread_set_priority(handle,priority);
}
