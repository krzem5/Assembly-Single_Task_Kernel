#include <user/syscall.h>
#include <user/types.h>



void system_shutdown(u8 flags){
	_syscall_system_shutdown(flags);
}
