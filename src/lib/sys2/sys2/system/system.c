#include <sys2/clock/clock.h>
#include <sys2/error/error.h>
#include <sys2/io/io.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/time/time.h>
#include <sys2/types.h>



static _Bool _sys2_initialized=0;



static void SYS2_CONSTRUCTOR _execute_init(void){
	if (_sys2_initialized){
		return;
	}
	_sys2_initialized=1;
	__sys2_clock_init();
	__sys2_io_init();
	__sys2_time_init();
}



SYS2_PUBLIC void __sys2_init(void){
	_execute_init();
}



SYS2_PUBLIC sys2_error_t sys2_system_shutdown(u32 flags){
	return _sys2_syscall_system_shutdown(flags);
}
