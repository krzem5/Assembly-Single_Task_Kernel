#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#include <opengl/syscall.h>
#include <ui/common.h>
#include <ui/framebuffer.h>
#define KERNEL_LOG_NAME "opengl_syscall"



static error_t _syscall_get_driver_instance(u16 min_version){
	opengl_driver_instance_t* best=NULL;
	HANDLE_FOREACH(opengl_driver_instance_handle_type){
		opengl_driver_instance_t* instance=handle->object;
		if (instance->driver->opengl_version>=min_version){
			best=instance;
			min_version=instance->driver->opengl_version;
		}
	}
	return (best?best->handle.rb_node.key:0);
}



static error_t _syscall_get_driver_instance_data(opengl_user_driver_instance_t instance,KERNEL_USER_POINTER opengl_user_driver_instance_data_t* buffer,u64 buffer_length){
	if (buffer_length<sizeof(opengl_user_driver_instance_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* driver_instance_handle=handle_lookup_and_acquire(instance,opengl_driver_instance_handle_type);
	if (!driver_instance_handle){
		return ERROR_INVALID_HANDLE;
	}
	opengl_driver_instance_t* driver_instance=driver_instance_handle->object;
	buffer->opengl_version=driver_instance->driver->opengl_version;
	str_copy(driver_instance->driver->name,(char*)(buffer->driver_name),32);
	str_copy(driver_instance->renderer,(char*)(buffer->renderer_name),64);
	str_copy(driver_instance->driver->library,(char*)(buffer->library),128);
	handle_release(driver_instance_handle);
	return ERROR_OK;
}



static error_t _syscall_create_state(opengl_user_driver_instance_t instance){
	handle_t* driver_instance_handle=handle_lookup_and_acquire(instance,opengl_driver_instance_handle_type);
	if (!driver_instance_handle){
		return ERROR_INVALID_HANDLE;
	}
	opengl_driver_instance_t* driver_instance=driver_instance_handle->object;
	opengl_state_t* state=opengl_create_state(driver_instance);
	handle_release(driver_instance_handle);
	return (state?state->handle.rb_node.key:ERROR_NO_MEMORY);
}



static error_t _syscall_delete_state(opengl_user_state_t state){
	handle_t* state_handle=handle_lookup_and_acquire(state,opengl_state_handle_type);
	if (!state_handle){
		return ERROR_INVALID_HANDLE;
	}
	handle_release(state_handle);
	opengl_delete_state(state_handle->object);
	return ERROR_OK;
}



static error_t _syscall_set_state_framebuffer(opengl_user_state_t state_handle_id,handle_id_t framebuffer_handle_id){
	handle_t* state_handle=handle_lookup_and_acquire(state_handle_id,opengl_state_handle_type);
	if (!state_handle){
		return ERROR_INVALID_HANDLE;
	}
	ui_framebuffer_t* framebuffer=NULL;
	if (framebuffer_handle_id){
		handle_t* framebuffer_handle=handle_lookup_and_acquire(framebuffer_handle_id,ui_framebuffer_handle_type);
		if (!framebuffer_handle){
			handle_release(state_handle);
			return ERROR_INVALID_HANDLE;
		}
		if (ui_common_get_process()!=THREAD_DATA->process->handle.rb_node.key&&!(acl_get(framebuffer_handle->acl,THREAD_DATA->process)&UI_FRAMEBUFFER_ACL_FLAG_USE)){
			handle_release(framebuffer_handle);
			return ERROR_DENIED;
		}
		framebuffer=framebuffer_handle->object;
	}
	opengl_state_t* state=state_handle->object;
	ui_framebuffer_t* old_framebuffer=state->framebuffer;
	state->framebuffer=framebuffer;
	state->driver_instance->driver->update_render_target(state->driver_instance,state);
	if (old_framebuffer){
		handle_release(&(old_framebuffer->handle));
	}
	handle_release(state_handle);
	return ERROR_OK;
}



static error_t _syscall_flush_command_buffer(opengl_user_state_t state_handle_id,KERNEL_USER_POINTER void* buffer,u32 buffer_length){
	if (buffer_length&(sizeof(u32)-1)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* state_handle=handle_lookup_and_acquire(state_handle_id,opengl_state_handle_type);
	if (!state_handle){
		return ERROR_INVALID_HANDLE;
	}
	opengl_state_t* state=state_handle->object;
	if (!(acl_get(state->handle.acl,THREAD_DATA->process)&OPENGL_STATE_ACL_FLAG_SEND_COMMANDS)){
		handle_release(state_handle);
		return ERROR_DENIED;
	}
	if (!state->framebuffer){
		handle_release(state_handle);
		return ERROR_INVALID_ARGUMENT(0);
	}
	state->driver_instance->driver->process_commands(state->driver_instance,state,buffer,buffer_length);
	handle_release(state_handle);
	return ERROR_OK;
}



static syscall_callback_t const _opengl_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_driver_instance,
	[2]=(syscall_callback_t)_syscall_get_driver_instance_data,
	[3]=(syscall_callback_t)_syscall_create_state,
	[4]=(syscall_callback_t)_syscall_delete_state,
	[5]=(syscall_callback_t)_syscall_set_state_framebuffer,
	[6]=(syscall_callback_t)_syscall_flush_command_buffer,
};



void opengl_syscall_init(void){
	LOG("Initializing OpenGL syscalls...");
	syscall_create_table("opengl",_opengl_syscall_functions,sizeof(_opengl_syscall_functions)/sizeof(syscall_callback_t));
}
