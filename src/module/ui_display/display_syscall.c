#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/common.h>
#include <ui/display.h>
#include <ui/display_syscall.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "ui_display_syscall"



static error_t _syscall_get_next_display(handle_id_t display_handle_id){
	handle_descriptor_t* display_handle_descriptor=handle_get_descriptor(ui_display_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(display_handle_descriptor->tree),(display_handle_id?display_handle_id+1:0));
	return (rb_node?rb_node->key:0);
}



static error_t _syscall_get_display_data(handle_id_t display_handle_id,ui_display_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(ui_display_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length(buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* display_handle=handle_lookup_and_acquire(display_handle_id,ui_display_handle_type);
	if (!display_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_display_t* display=display_handle->object;
	buffer->index=display->index;
	if (display->mode){
		buffer->mode.width=display->mode->width;
		buffer->mode.height=display->mode->height;
		buffer->mode.freq=display->mode->freq;
	}
	else{
		buffer->mode.width=0;
		buffer->mode.height=0;
		buffer->mode.freq=0;
	}
	handle_release(display_handle);
	return ERROR_OK;
}



static error_t _syscall_get_display_info(handle_id_t display_handle_id,ui_display_user_info_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(ui_display_user_info_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length(buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* display_handle=handle_lookup_and_acquire(display_handle_id,ui_display_handle_type);
	if (!display_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_display_t* display=display_handle->object;
	const ui_display_info_t* display_info=display->display_info;
	memcpy(buffer->manufacturer,display_info->manufacturer,sizeof(buffer->manufacturer));
	buffer->manufacturer_product_code=display_info->manufacturer_product_code;
	buffer->serial_number=display_info->serial_number;
	buffer->video_interface=display_info->video_interface;
	buffer->screen_width_cm=display_info->screen_width_cm;
	buffer->screen_height_cm=display_info->screen_height_cm;
	memcpy(buffer->name,display_info->name,sizeof(buffer->name));
	u32 buffer_mode_count=(buffer_length-sizeof(ui_display_user_info_t))/sizeof(ui_display_user_mode_t);
	u32 mode_count=0;
	for (const ui_display_info_mode_t* mode=display_info->modes;mode;mode=mode->next){
		if (mode_count<buffer_mode_count){
			buffer->modes[mode_count].width=mode->width;
			buffer->modes[mode_count].height=mode->height;
			buffer->modes[mode_count].freq=mode->freq;
		}
		mode_count++;
	}
	buffer->mode_count=mode_count;
	handle_release(display_handle);
	return ERROR_OK;
}



static error_t _syscall_get_display_framebuffer(handle_id_t display_handle_id){
	handle_t* display_handle=handle_lookup_and_acquire(display_handle_id,ui_display_handle_type);
	if (!display_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_display_t* display=display_handle->object;
	u64 out=(display->framebuffer?display->framebuffer->handle.rb_node.key:0);
	handle_release(display_handle);
	return out;
}



static error_t _syscall_get_display_framebuffer2(handle_id_t display_handle_id){
	handle_t* display_handle=handle_lookup_and_acquire(display_handle_id,ui_display_handle_type);
	if (!display_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_display_t* display=display_handle->object;
	u64 out=(display->framebuffer2?display->framebuffer2->handle.rb_node.key:0);
	handle_release(display_handle);
	return out;
}



static error_t _syscall_get_framebuffer_config(handle_id_t framebuffer_handle_id,ui_display_user_framebuffer_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(ui_display_user_framebuffer_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length(buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* framebuffer_handle=handle_lookup_and_acquire(framebuffer_handle_id,ui_framebuffer_handle_type);
	if (!framebuffer_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_framebuffer_t* framebuffer=framebuffer_handle->object;
	buffer->width=framebuffer->width;
	buffer->height=framebuffer->height;
	buffer->format=framebuffer->format;
	handle_release(framebuffer_handle);
	return ERROR_OK;
}



static error_t _syscall_get_framebuffer2_config(handle_id_t framebuffer_handle_id,ui_display_user_framebuffer_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(ui_display_user_framebuffer_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length(buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* framebuffer_handle=handle_lookup_and_acquire(framebuffer_handle_id,ui_framebuffer2_handle_type);
	if (!framebuffer_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_framebuffer2_t* framebuffer=framebuffer_handle->object;
	buffer->width=framebuffer->width;
	buffer->height=framebuffer->height;
	buffer->format=framebuffer->format;
	handle_release(framebuffer_handle);
	return ERROR_OK;
}



static error_t _syscall_flush_display_framebuffer(handle_id_t display_handle_id){
	if (ui_common_get_process()!=THREAD_DATA->process->handle.rb_node.key){
		return ERROR_DENIED;
	}
	handle_t* display_handle=handle_lookup_and_acquire(display_handle_id,ui_display_handle_type);
	if (!display_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_display_t* display=display_handle->object;
	if (display->framebuffer){
		display->driver->flush_framebuffer(display);
	}
	handle_release(display_handle);
	return ERROR_OK;
}



static syscall_callback_t const _ui_display_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_next_display,
	[2]=(syscall_callback_t)_syscall_get_display_data,
	[3]=(syscall_callback_t)_syscall_get_display_info,
	[4]=(syscall_callback_t)_syscall_get_display_framebuffer,
	[5]=(syscall_callback_t)_syscall_get_framebuffer_config,
	[7]=(syscall_callback_t)_syscall_flush_display_framebuffer,
	[8]=(syscall_callback_t)_syscall_get_display_framebuffer2,
	[9]=(syscall_callback_t)_syscall_get_framebuffer2_config,
};



void ui_display_syscall_init(void){
	LOG("Initializing UI display syscalls...");
	syscall_create_table("ui_display",_ui_display_syscall_functions,sizeof(_ui_display_syscall_functions)/sizeof(syscall_callback_t));
}
