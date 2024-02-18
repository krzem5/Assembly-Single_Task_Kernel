#include <sys/error/error.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_error_t sys_pipe_create(const void* path){
	return _sys_syscall_pipe_create(path);
}
