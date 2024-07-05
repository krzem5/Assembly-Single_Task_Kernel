#include <sys/error/error.h>
#include <sys/memory/memory.h>
#include <sys/mp/event.h>
#include <sys/mp/thread.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



#define DEFAULT_STACK_SIZE 0x200000



static void _thread_bootstrap(u64 func_and_flags,void* arg,u64 stack_top){
	void* ret=((void* (*)(void*))(func_and_flags&0x7fffffffffffffffull))(arg);
	if (func_and_flags>>63){
		// cannot be executed; it deletes the stack of the current function
		// sys_memory_unmap((void*)(stack_top-DEFAULT_STACK_SIZE),DEFAULT_STACK_SIZE);
	}
	_sys_syscall_thread_stop(0,ret);
}



SYS_PUBLIC u64 sys_thread_await_event(sys_event_t event){
	return sys_thread_await_events(&event,1);
}



SYS_PUBLIC u64 sys_thread_await_events(const sys_event_t* events,u32 count){
	return _sys_syscall_thread_await_events(events,count);
}



SYS_PUBLIC sys_thread_t sys_thread_create(void* func,void* arg,void* stack){
	u64 func_and_flags=(u64)func;
	if (func_and_flags>>63){
		return SYS_ERROR_INVALID_ARGUMENT(0);
	}
	if (!stack){
		stack=(void*)sys_memory_map(DEFAULT_STACK_SIZE,SYS_MEMORY_FLAG_STACK|SYS_MEMORY_FLAG_WRITE|SYS_MEMORY_FLAG_READ,0);
		if (SYS_IS_ERROR(stack)){
			return (u64)stack;
		}
		stack+=DEFAULT_STACK_SIZE;
		func_and_flags|=0x8000000000000000ull;
	}
	// '-8' aligns stack because of 'push rbp' at the beginning of _thread_bootstrap
	return _sys_syscall_thread_create((u64)_thread_bootstrap,func_and_flags,(u64)arg,(u64)stack,((u64)stack)-8);
}



SYS_PUBLIC sys_thread_priority_t sys_thread_get_priority(sys_thread_t thread){
	return _sys_syscall_thread_get_priority(thread);
}



SYS_PUBLIC sys_thread_t sys_thread_get_handle(void){
	return _sys_syscall_thread_get_tid();
}



SYS_PUBLIC sys_error_t sys_thread_set_priority(sys_thread_t thread,sys_thread_priority_t priority){
	return _sys_syscall_thread_set_priority(thread,priority);
}



SYS_PUBLIC sys_error_t sys_thread_start(sys_thread_t thread){
	return _sys_syscall_thread_start(thread);
}



SYS_PUBLIC sys_error_t sys_thread_stop(sys_thread_t thread,void* return_value){
	return _sys_syscall_thread_stop(thread,return_value);
}



SYS_PUBLIC void* sys_thread_get_return_value(sys_thread_t thread){
	return (void*)_sys_syscall_thread_get_return_value(thread);
}



SYS_PUBLIC sys_thread_t sys_thread_iter_start(sys_process_t process){
	return _sys_syscall_thread_iter(process,0);
}



SYS_PUBLIC sys_thread_t sys_thread_iter_next(sys_process_t process,sys_thread_t thread){
	return _sys_syscall_thread_iter(process,thread);
}



SYS_PUBLIC sys_error_t sys_thread_query(sys_thread_t thread,sys_thread_query_result_t* out){
	return _sys_syscall_thread_query(thread,out,sizeof(sys_thread_query_result_t));
}
