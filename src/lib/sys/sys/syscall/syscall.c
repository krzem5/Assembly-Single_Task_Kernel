#include <sys/error/error.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



SYS_PUBLIC sys_error_t sys_syscall_get_table_offset(const char* name){
	return _sys_syscall_syscall_table_get_offset(name);
}
