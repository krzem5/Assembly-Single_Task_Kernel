#ifndef _OPENGL_OPENGL_H_
#define _OPENGL_OPENGL_H_ 1
#include <sys/types.h>
#include <ui/display.h>



typedef u64 opengl_driver_instance_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
} opengl_driver_instance_data_t;



typedef u64 opengl_state_t;



extern opengl_state_t opengl_current_state;



void opengl_init(void);



opengl_state_t opengl_create_state(u16 min_version);



_Bool opengl_set_state(opengl_state_t state);



_Bool opengl_set_state_framebuffer(opengl_state_t state,ui_framebuffer_handle_t framebuffer);



#endif
