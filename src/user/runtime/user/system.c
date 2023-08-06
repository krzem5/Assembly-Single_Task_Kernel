#include <user/syscall.h>
#include <user/system.h>
#include <user/types.h>



system_data_t system_data;



void _system_init(void){
	_syscall_system_config(&system_data,sizeof(system_data_t));
}



void system_shutdown(u8 flags){
	_syscall_system_shutdown(flags);
}
