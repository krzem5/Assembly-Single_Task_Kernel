#include <opengl/command_buffer.h>
#include <opengl/opengl.h>
#include <opengl/syscalls.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <ui/display.h>



static _Bool _opengl_initialized=0;

opengl_state_t opengl_current_state=0;



SYS_PUBLIC void opengl_init(void){
	if (!opengl_syscalls_init()){
		printf("OpenGL module is not loaded\n");
		return;
	}
	opengl_command_buffer_init();
	printf("OpenGL initialized\n");
	_opengl_initialized=1;
}



SYS_PUBLIC opengl_state_t opengl_create_state(u16 min_version){
	if (!_opengl_initialized){
		return 0;
	}
	opengl_driver_instance_t driver_instance=opengl_syscall_get_driver_instance(min_version);
	opengl_driver_instance_data_t driver_instance_data;
	if (!driver_instance||!opengl_syscall_get_driver_instance_data(driver_instance,&driver_instance_data)){
		return 0;
	}
	printf("Driver: %s/%s, %u\n",driver_instance_data.driver_name,driver_instance_data.renderer_name,driver_instance_data.opengl_version);
	return opengl_syscall_create_state(driver_instance);
}



SYS_PUBLIC _Bool opengl_set_state(opengl_state_t state){
	if (!_opengl_initialized){
		return 0;
	}
	opengl_command_buffer_flush();
	if (!opengl_syscall_set_state(state)){
		opengl_current_state=0;
		return 0;
	}
	opengl_current_state=state;
	return 1;
}



SYS_PUBLIC _Bool opengl_set_state_framebuffer(opengl_state_t state,ui_framebuffer_handle_t framebuffer){
	if (!_opengl_initialized){
		return 0;
	}
	opengl_command_buffer_flush();
	return opengl_syscall_set_state_framebuffer(state,framebuffer);
}
