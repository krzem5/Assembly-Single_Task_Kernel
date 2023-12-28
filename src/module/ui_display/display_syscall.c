#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
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
	panic("_syscall_get_display_info");
}



static error_t _syscall_flush_framebuffer(handle_id_t display_handle_id,u64 address,ui_display_user_framebuffer_t* current_config){
	if (!process_is_root()){
		return ERROR_DENIED;
	}
	if (syscall_get_user_pointer_max_length(current_config)<sizeof(ui_display_user_framebuffer_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	handle_t* display_handle=handle_lookup_and_acquire(display_handle_id,ui_display_handle_type);
	if (!display_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_display_t* display=display_handle->object;
	if (address&&(!display->framebuffer||(display->framebuffer->width!=current_config->width||display->framebuffer->height!=current_config->height||display->framebuffer->format!=current_config->format))){
		mmap_dealloc(&(THREAD_DATA->process->mmap),address,current_config->size);
		address=0;
		current_config->size=0;
		current_config->width=0;
		current_config->height=0;
		current_config->format=UI_FRAMEBUFFER_FORMAT_NONE;
	}
	if (!display->framebuffer){
		handle_release(display_handle);
		return ERROR_OK;
	}
	if (address){
		display->driver->flush_framebuffer(display);
		handle_release(display_handle);
		return ERROR_OK;
	}
	if (display->framebuffer->_is_user_mapped){
		return ERROR_DENIED;
	}
	current_config->size=display->framebuffer->size;
	current_config->width=display->framebuffer->width;
	current_config->height=display->framebuffer->height;
	current_config->format=display->framebuffer->format;
	mmap_region_t* region=mmap_alloc(&(THREAD_DATA->process->mmap),0,display->framebuffer->size,NULL,MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER|MMAP_REGION_FLAG_VMM_NOEXECUTE,NULL,display->framebuffer->address);
	if (region){
		display->framebuffer->_is_user_mapped=1;
	}
	handle_release(display_handle);
	return (region?region->rb_node.key:ERROR_NO_MEMORY);
}



static syscall_callback_t const _ui_display_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_next_display,
	[2]=(syscall_callback_t)_syscall_get_display_data,
	[3]=(syscall_callback_t)_syscall_get_display_info,
	[4]=(syscall_callback_t)_syscall_flush_framebuffer,
};



void ui_display_syscall_init(void){
	LOG("Initializing UI display syscalls...");
	syscall_create_table("ui_display",_ui_display_syscall_functions,sizeof(_ui_display_syscall_functions)/sizeof(syscall_callback_t));
}
