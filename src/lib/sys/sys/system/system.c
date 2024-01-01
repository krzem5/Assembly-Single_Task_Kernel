#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/io/io.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/time/time.h>
#include <sys/types.h>



static _Bool _sys_initialized=0;



static void SYS_CONSTRUCTOR _execute_init(void){
	if (_sys_initialized){
		return;
	}
	_sys_initialized=1;
	__sys_clock_init();
	__sys_io_init();
	__sys_time_init();
}



SYS_PUBLIC void __sys_init(void){
	_execute_init();
}



SYS_PUBLIC sys_error_t sys_system_shutdown(u32 flags){
	return _sys_syscall_system_shutdown(flags);
}
