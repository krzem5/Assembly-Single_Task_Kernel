#include <user/syscall.h>
#include <user/types.h>



void shutdown(u8 flags){
	_syscall_system_shutdown(flags);
}
