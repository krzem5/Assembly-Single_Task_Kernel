#include <user/syscall.h>
#include <user/types.h>



void __attribute__((noreturn)) thread_stop(void){
	_syscall_thread_stop();
}



u32 __attribute__((returns_twice)) thread_create(u32 stack_size){
	return _syscall_thread_create(stack_size);
}
