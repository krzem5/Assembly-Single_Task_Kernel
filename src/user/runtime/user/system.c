#include <user/syscall.h>
#include <user/system.h>
#include <user/types.h>



void save_context(void){
	_syscall_context_save();
}



void shutdown(u8 flags){
	if (flags&SHUTDOWN_FLAG_SAVE_CONTEXT){
		_syscall_context_save();
	}
	_syscall_system_shutdown(flags);
}
