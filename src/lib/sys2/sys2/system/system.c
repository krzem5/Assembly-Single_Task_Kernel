#include <sys2/error/error.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



sys2_error_t sys2_system_shutdown(u32 flags){
	return _sys2_syscall_system_shutdown(flags);
}
