#include <sys2/error/error.h>
#include <sys2/syscall/kernel_syscalls.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_error_t sys2_syscall_get_table_offset(const char* name){
	return _syscall_syscall_table_get_offset(name);
}
