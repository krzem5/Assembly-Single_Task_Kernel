#include <sys/error/error.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <ui/display.h>



static u64 _ui_display_syscall_offset=0;



static void SYS_CONSTRUCTOR _init(void){
	_ui_display_syscall_offset=sys_syscall_get_table_offset("ui_display");
}



SYS_PUBLIC ui_display_handle_t ui_display_iter_start(void){
	return _sys_syscall1(_ui_display_syscall_offset|0x00000001,0);
}



SYS_PUBLIC ui_display_handle_t ui_display_iter_next(ui_display_handle_t handle){
	return _sys_syscall1(_ui_display_syscall_offset|0x00000001,handle);
}



SYS_PUBLIC u64 ui_display_get_data(ui_display_handle_t handle,ui_display_data_t* out){
	return _sys_syscall3(_ui_display_syscall_offset|0x00000002,handle,(u64)out,sizeof(ui_display_data_t));
}



SYS_PUBLIC u64 ui_display_get_info(ui_display_handle_t handle,ui_display_info_t* buffer,u32 buffer_length){
	return _sys_syscall3(_ui_display_syscall_offset|0x00000003,handle,(u64)buffer,buffer_length);
}



SYS_PUBLIC u64 ui_display_get_display_framebuffer(ui_display_handle_t handle){
	return _sys_syscall1(_ui_display_syscall_offset|0x00000004,handle);
}



SYS_PUBLIC u64 ui_display_get_framebuffer_config(ui_framebuffer_handle_t handle,ui_display_framebuffer_t* out){
	return _sys_syscall3(_ui_display_syscall_offset|0x00000005,handle,(u64)out,sizeof(ui_display_framebuffer_t));
}



SYS_PUBLIC u64 ui_display_flush_display_framebuffer(ui_display_handle_t handle){
	return _sys_syscall1(_ui_display_syscall_offset|0x00000006,handle);
}
