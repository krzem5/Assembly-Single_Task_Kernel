#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/display.h>
#include <ui/display_syscall.h>
#define KERNEL_LOG_NAME "ui_display_syscall"



static error_t _syscall_get_next_display(handle_id_t display_handle){
	handle_descriptor_t* display_handle_descriptor=handle_get_descriptor(ui_display_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(display_handle_descriptor->tree),(display_handle?display_handle+1:0));
	return (rb_node?rb_node->key:0);
}



static error_t _syscall_get_display_data(handle_id_t display_handle,ui_display_user_data_t* buffer,u32 buffer_length){
	panic("_syscall_get_display_data");
}



static error_t _syscall_get_display_info(handle_id_t display_handle,ui_display_user_info_t* buffer,u32 buffer_length){
	panic("_syscall_get_display_info");
}



static error_t _syscall_map_framebuffer(handle_id_t display_handle){
	panic("_syscall_map_framebuffer");
}



static syscall_callback_t const _ui_display_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_next_display,
	[2]=(syscall_callback_t)_syscall_get_display_data,
	[3]=(syscall_callback_t)_syscall_get_display_info,
	[4]=(syscall_callback_t)_syscall_map_framebuffer,
};



void ui_display_syscall_init(void){
	LOG("Initializing UI display syscalls...");
	syscall_create_table("ui_display",_ui_display_syscall_functions,sizeof(_ui_display_syscall_functions)/sizeof(syscall_callback_t));
}
