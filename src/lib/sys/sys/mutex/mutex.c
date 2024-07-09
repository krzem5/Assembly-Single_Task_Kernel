#include <sys/error/error.h>
#include <sys/mutex/mutex.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_mutex_t sys_mutex_create(void){
	return _sys_syscall_mutex_create();
}



SYS_PUBLIC sys_error_t sys_mutex_delete(sys_mutex_t mutex){
	return _sys_syscall_mutex_delete(mutex);
}



SYS_PUBLIC sys_error_t sys_mutex_get_holder(sys_mutex_t mutex){
	return _sys_syscall_mutex_get_holder(mutex);
}



SYS_PUBLIC sys_error_t sys_mutex_acquire(sys_mutex_t mutex){
	return _sys_syscall_mutex_acquire(mutex);
}



SYS_PUBLIC sys_error_t sys_mutex_release(sys_mutex_t mutex){
	return _sys_syscall_mutex_release(mutex);
}
