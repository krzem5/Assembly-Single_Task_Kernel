#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/time/time.h>
#include <sys/types.h>



extern void __sys_linker_execute_fini(void) __attribute__((weak));



static _Bool _sys_initialized=0;
static _Bool _sys_deinitialized=0;



static void SYS_CONSTRUCTOR _execute_init(void){
	if (_sys_initialized){
		return;
	}
	_sys_initialized=1;
	__sys_clock_init();
	__sys_heap_init();
	__sys_io_init();
}



static void SYS_DESTRUCTOR _execute_fini(void){
	if (_sys_deinitialized){
		return;
	}
	_sys_deinitialized=1;
}



SYS_PUBLIC void __sys_init(void){
	_execute_init();
}



SYS_PUBLIC void __sys_fini(void){
	__sys_linker_execute_fini();
}



SYS_PUBLIC sys_error_t sys_system_shutdown(u32 flags){
	return _sys_syscall_system_shutdown(flags);
}
