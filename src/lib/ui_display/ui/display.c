#include <sys/error.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <sys/syscall_generic.h>
#include <sys/types.h>
#include <ui/display.h>



static u64 _ui_display_syscall_offset=0;



SYS_PUBLIC ui_display_handle_t ui_display_iter_start(void){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=_syscall_syscall_table_get_offset("ui_display");
	}
	return _syscall1(_ui_display_syscall_offset|0x00000001,0);
}



SYS_PUBLIC ui_display_handle_t ui_display_iter_next(ui_display_handle_t handle){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=_syscall_syscall_table_get_offset("ui_display");
	}
	return _syscall1(_ui_display_syscall_offset|0x00000001,handle);
}



SYS_PUBLIC u64 ui_display_get_data(ui_display_handle_t handle,ui_display_data_t* out){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=_syscall_syscall_table_get_offset("ui_display");
	}
	return _syscall3(_ui_display_syscall_offset|0x00000002,handle,(u64)out,sizeof(ui_display_data_t));
}



SYS_PUBLIC u64 ui_display_get_info(ui_display_handle_t handle,ui_display_info_t* buffer,u32 buffer_length){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=_syscall_syscall_table_get_offset("ui_display");
	}
	return _syscall3(_ui_display_syscall_offset|0x00000003,handle,(u64)buffer,buffer_length);
}



SYS_PUBLIC u64 ui_display_flush_framebuffer(ui_display_handle_t handle,const void* address,ui_display_framebuffer_t* config){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=_syscall_syscall_table_get_offset("ui_display");
	}
	return _syscall3(_ui_display_syscall_offset|0x00000004,handle,(u64)address,(u64)config);
}

