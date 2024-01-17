#include <opengl/_internal/state.h>
#include <opengl/opengl.h>
#include <sys/error/error.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <ui/display.h>



static u64 _opengl_syscall_offset=0xffffffffffffffffull;



static void SYS_CONSTRUCTOR _init(void){
	u64 offset=sys_syscall_get_table_offset("opengl");
	if (!SYS_IS_ERROR(offset)){
		_opengl_syscall_offset=offset;
	}
}



_Bool opengl_syscalls_init(void){
	if (_opengl_syscall_offset==0xffffffffffffffffull){
		_init();
	}
	return _opengl_syscall_offset!=0xffffffffffffffffull;
}



opengl_driver_instance_t opengl_syscall_get_driver_instance(u16 min_version){
	return _sys_syscall1(_opengl_syscall_offset|0x00000001,min_version);
}



_Bool opengl_syscall_get_driver_instance_data(opengl_driver_instance_t instance,opengl_driver_instance_data_t* out){
	return _sys_syscall3(_opengl_syscall_offset|0x00000002,instance,(u64)out,sizeof(opengl_driver_instance_data_t))==SYS_ERROR_OK;
}



opengl_state_id_t opengl_syscall_create_state(opengl_driver_instance_t instance){
	u64 out=_sys_syscall1(_opengl_syscall_offset|0x00000003,instance);
	return (SYS_IS_ERROR(out)?0:out);
}



_Bool opengl_syscall_delete_state(opengl_state_id_t state){
	return _sys_syscall1(_opengl_syscall_offset|0x00000004,state)==SYS_ERROR_OK;
}



_Bool opengl_syscall_set_state_framebuffer(opengl_state_id_t state,ui_framebuffer_handle_t framebuffer){
	return _sys_syscall2(_opengl_syscall_offset|0x00000005,state,framebuffer)==SYS_ERROR_OK;
}



void opengl_syscall_flush_command_buffer(void* buffer,u32 buffer_size){
	_sys_syscall3(_opengl_syscall_offset|0x00000006,opengl_current_state_id,(u64)buffer,buffer_size);
}
