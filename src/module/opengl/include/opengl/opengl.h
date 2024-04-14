#ifndef _OPENGL_OPENGL_H_
#define _OPENGL_OPENGL_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>
#include <ui/framebuffer.h>



#define OPENGL_STATE_ACL_FLAG_SEND_COMMANDS 1



typedef struct _OPENGL_STATE{
	handle_t handle;
	struct _OPENGL_DRIVER_INSTANCE* driver_instance;
	void* ctx;
	ui_framebuffer_t* framebuffer;
} opengl_state_t;



typedef struct _OPENGL_DRIVER{
	const char* name;
	const char* library;
	u16 opengl_version;
	_Bool (*init_state)(struct _OPENGL_DRIVER_INSTANCE*,opengl_state_t*);
	void (*deinit_state)(struct _OPENGL_DRIVER_INSTANCE*,opengl_state_t*);
	void (*update_render_target)(struct _OPENGL_DRIVER_INSTANCE*,opengl_state_t*);
	void (*process_commands)(struct _OPENGL_DRIVER_INSTANCE*,opengl_state_t*,KERNEL_USER_POINTER void* command_buffer,u32 command_buffer_size);
} opengl_driver_t;



typedef struct _OPENGL_DRIVER_INSTANCE{
	handle_t handle;
	char renderer[64];
	const opengl_driver_t* driver;
	void* ctx;
} opengl_driver_instance_t;



extern handle_type_t opengl_driver_instance_handle_type;
extern handle_type_t opengl_state_handle_type;



opengl_driver_instance_t* opengl_create_driver_instance(const opengl_driver_t* driver,const char* renderer,void* ctx);



opengl_state_t* opengl_create_state(opengl_driver_instance_t* driver_instance);



void opengl_delete_state(opengl_state_t* state);



#endif
