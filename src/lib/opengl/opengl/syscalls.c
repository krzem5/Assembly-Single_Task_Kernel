#include <opengl/syscalls.h>
#include <sys/syscall.h>
#include <sys/syscall_generic.h>
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



opengl_driver_instance_t opengl_syscall_get_driver_instance(u16 min_version){
	return _syscall1(_opengl_syscall_offset|0x00000001,min_version);
}



_Bool opengl_syscall_get_driver_instance_data(opengl_driver_instance_t instance,opengl_driver_instance_data_t* out){
	return _syscall3(_opengl_syscall_offset|0x00000002,instance,(u64)out,sizeof(opengl_driver_instance_data_t))==0;
}
