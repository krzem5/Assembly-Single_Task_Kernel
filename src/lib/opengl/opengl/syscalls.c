#include <opengl/syscalls.h>
#include <sys/syscall.h>
#include <sys/types.h>



static u64 _opengl_syscall_offset=0xffffffffffffffffull;



_Bool opengl_syscalls_init(void){
	s64 offset=_syscall_syscall_table_get_offset("opengl");
	if (offset<0){
		return 0;
	}
	_opengl_syscall_offset=offset;
	return 1;
}
