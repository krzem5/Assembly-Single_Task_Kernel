#include <sys/error/error.h>
#include <sys/mp/event.h>
#include <sys/mp/timer.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_timer_t sys_timer_create(u64 interval,u64 count){
	return _sys_syscall_timer_create(interval,count);
}



SYS_PUBLIC sys_error_t sys_timer_delete(sys_timer_t timer){
	return _sys_syscall_timer_delete(timer);
}



SYS_PUBLIC u64 sys_timer_get_deadline(sys_timer_t timer){
	return _sys_syscall_timer_get_deadline(timer);
}



SYS_PUBLIC sys_event_t sys_timer_get_event(sys_timer_t timer){
	return _sys_syscall_timer_get_event(timer);
}



SYS_PUBLIC u64 sys_timer_update(sys_timer_t timer,u64 interval,u64 count){
	return _sys_syscall_timer_update(timer,interval,count);
}
