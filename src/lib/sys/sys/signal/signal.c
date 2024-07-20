#include <sys/error/error.h>
#include <sys/mp/event.h>
#include <sys/signal/signal.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



extern void _sys_signal_handler_wrapper(void);



SYS_PUBLIC sys_event_t sys_signal_get_event(void){
	return _sys_syscall_signal_get_event();
}



SYS_PUBLIC sys_error_t sys_signal_get_signal(void){
	return _sys_syscall_signal_get_signal();
}



SYS_PUBLIC sys_signal_mask_t sys_signal_get_pending_signals(bool is_process){
	return _sys_syscall_signal_get_pending_signals(is_process);
}



SYS_PUBLIC sys_signal_mask_t sys_signal_get_mask(bool is_process_mask){
	return _sys_syscall_signal_get_mask(is_process_mask);
}



SYS_PUBLIC sys_error_t sys_signal_set_mask(sys_signal_mask_t mask,bool is_process_mask){
	return _sys_syscall_signal_set_mask(mask,is_process_mask);
}



SYS_PUBLIC sys_error_t sys_signal_set_handler(void* handler){
	return _sys_syscall_signal_set_handler(_sys_signal_handler_wrapper,(u64)handler);
}



SYS_PUBLIC sys_error_t sys_signal_dispatch(u64 handle,sys_signal_t signal){
	return _sys_syscall_signal_dispatch(handle,signal);
}
