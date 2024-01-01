#include <sys2/error/error.h>
#include <sys2/syscall/syscall.h>
#include <sys2/types.h>
#include <ui/display.h>



static u64 _ui_display_syscall_offset=0;



SYS2_PUBLIC ui_display_handle_t ui_display_iter_start(void){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall1(_ui_display_syscall_offset|0x00000001,0);
}



SYS2_PUBLIC ui_display_handle_t ui_display_iter_next(ui_display_handle_t handle){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall1(_ui_display_syscall_offset|0x00000001,handle);
}



SYS2_PUBLIC u64 ui_display_get_data(ui_display_handle_t handle,ui_display_data_t* out){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall3(_ui_display_syscall_offset|0x00000002,handle,(u64)out,sizeof(ui_display_data_t));
}



SYS2_PUBLIC u64 ui_display_get_info(ui_display_handle_t handle,ui_display_info_t* buffer,u32 buffer_length){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall3(_ui_display_syscall_offset|0x00000003,handle,(u64)buffer,buffer_length);
}



SYS2_PUBLIC u64 ui_display_get_display_framebuffer(ui_display_handle_t handle){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall1(_ui_display_syscall_offset|0x00000004,handle);
}



SYS2_PUBLIC u64 ui_display_get_framebuffer_config(ui_framebuffer_handle_t handle,ui_display_framebuffer_t* out){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall3(_ui_display_syscall_offset|0x00000005,handle,(u64)out,sizeof(ui_display_framebuffer_t));
}



SYS2_PUBLIC u64 ui_display_map_framebuffer(ui_framebuffer_handle_t handle){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall1(_ui_display_syscall_offset|0x00000006,handle);
}



SYS2_PUBLIC u64 ui_display_flush_display_framebuffer(ui_display_handle_t handle){
	if (!_ui_display_syscall_offset){
		_ui_display_syscall_offset=sys2_syscall_get_table_offset("ui_display");
	}
	return _sys2_syscall1(_ui_display_syscall_offset|0x00000007,handle);
}
