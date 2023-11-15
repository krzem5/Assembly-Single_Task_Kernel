#include <sys/syscall.h>
#include <sys/types.h>



SYS_PUBLIC void system_shutdown(u8 flags){
	_syscall_system_shutdown(flags);
}
