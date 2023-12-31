#include <sys/_kernel_syscall.h>
#include <sys/types.h>



SYS_PUBLIC void sys_system_shutdown(u8 flags){
	_syscall_system_shutdown(flags);
}
