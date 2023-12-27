#include <opengl/syscalls.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <sys/types.h>



static _Bool _opengl_initialized=0;



SYS_PUBLIC void opengl_init(void){
	if (!opengl_syscalls_init()){
		printf("OpenGL module is not loaded\n");
		return;
	}
	printf("OpenGL initialized\n");
	_opengl_initialized=1;
}



SYS_PUBLIC void opengl_create_state(u16 min_version){
	if (!_opengl_initialized){
		return;
	}
	opengl_driver_instance_t driver_instance=opengl_syscall_get_driver_instance(min_version);
	opengl_driver_instance_data_t driver_instance_data;
	if (!driver_instance||!opengl_syscall_get_driver_instance_data(driver_instance,&driver_instance_data)){
		return;
	}
	printf("Driver: %s/%s, %u\n",driver_instance_data.driver_name,driver_instance_data.renderer_name,driver_instance_data.opengl_version);
	opengl_state_t state=opengl_syscall_create_state(driver_instance);
	printf("State: %p\n",state);
}
