#include <sys/error/error.h>
#include <sys/handle/handle.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_error_t sys_handle_get_name(sys_handle_t handle,void* buffer,u32 buffer_size){
	return _sys_syscall_handle_get_name(handle,buffer,buffer_size);
}
