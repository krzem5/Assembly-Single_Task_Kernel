#ifndef _OPENGL_OPENGL_H_
#define _OPENGL_OPENGL_H_ 1
#include <GL/gl.h>
#include <sys2/types.h>
#include <ui/display.h>



typedef u64 opengl_driver_instance_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
} opengl_driver_instance_data_t;



typedef u64 opengl_state_id_t;



typedef void* opengl_state_t;



typedef struct _OPENGL_INTERNAL_STATE{
	opengl_state_id_t state_id;
	opengl_driver_instance_t driver_instance;
	u16 driver_opengl_version;
	char gl_renderer[64];
	char gl_shading_language_version[16];
	char gl_vendor[32];
	char gl_version[16];
	GLenum gl_error;
	GLuint gl_active_texture;
	GLfloat gl_clear_color_value[4];
	GLdouble gl_clear_depth_value;
	GLint gl_clear_stencil_value;
	GLint gl_viewport[4];
} opengl_internal_state_t;



extern opengl_state_id_t opengl_current_state_id;



void opengl_init(void);



opengl_state_t opengl_create_state(u16 min_version);



_Bool opengl_set_state(opengl_state_t state);



_Bool opengl_set_state_framebuffer(opengl_state_t state,ui_framebuffer_handle_t framebuffer);



#endif
