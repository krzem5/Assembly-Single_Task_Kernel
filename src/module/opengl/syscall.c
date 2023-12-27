#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#include <opengl/syscall.h>
#define KERNEL_LOG_NAME "opengl_syscalls"



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



static error_t _syscall_get_driver_instance_data(opengl_user_driver_instance_t instance,opengl_user_driver_instance_data_t* buffer,u64 buffer_length){
	if (buffer_length<sizeof(opengl_user_driver_instance_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length(buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* driver_instance_handle=handle_lookup_and_acquire(instance,opengl_driver_instance_handle_type);
	if (!driver_instance_handle){
		return ERROR_INVALID_HANDLE;
	}
	opengl_driver_instance_t* driver_instance=driver_instance_handle->object;
	buffer->opengl_version=driver_instance->driver->opengl_version;
	strcpy(buffer->driver_name,driver_instance->driver->name,32);
	strcpy(buffer->renderer_name,driver_instance->renderer,64);
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



static syscall_callback_t const _opengl_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_driver_instance,
	[2]=(syscall_callback_t)_syscall_get_driver_instance_data,
	[3]=(syscall_callback_t)_syscall_create_state,
	[4]=(syscall_callback_t)_syscall_delete_state,
};



void opengl_syscall_init(void){
	LOG("Initializing OpenGL syscalls...");
	syscall_create_table("opengl",_opengl_syscall_functions,sizeof(_opengl_syscall_functions)/sizeof(syscall_callback_t));
}
