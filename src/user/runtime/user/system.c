#include <user/syscall.h>
#include <user/types.h>



u32 system_get_string(u32 index,char* buffer,u32 size){
	return _syscall_system_get_string(index,buffer,size);
}



void system_shutdown(u8 flags){
	_syscall_system_shutdown(flags);
}
