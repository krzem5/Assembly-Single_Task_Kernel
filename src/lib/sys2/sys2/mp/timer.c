#include <sys2/error/error.h>
#include <sys2/mp/event.h>
#include <sys2/mp/timer.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_timer_t sys2_timer_create(u64 interval,u64 count){
	return _sys2_syscall_timer_create(interval,count);
}



SYS2_PUBLIC sys2_error_t sys2_timer_delete(sys2_timer_t timer){
	return _sys2_syscall_timer_delete(timer);
}



SYS2_PUBLIC u64 sys2_timer_get_deadline(sys2_timer_t timer){
	return _sys2_syscall_timer_get_deadline(timer);
}



SYS2_PUBLIC sys2_event_t sys2_timer_get_event(sys2_timer_t timer){
	return _sys2_syscall_timer_get_event(timer);
}



SYS2_PUBLIC u64 sys2_timer_update(sys2_timer_t timer,u64 interval,u64 count){
	return _sys2_syscall_timer_update(timer,interval,count);
}
