#ifndef _OPENGL__INTERNAL_STATE_H_
#define _OPENGL__INTERNAL_STATE_H_ 1
#include <GL/gl.h>
#include <glsl/backend.h>
#include <sys/types.h>



typedef u64 opengl_driver_instance_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
	char library[128];
} opengl_driver_instance_data_t;



typedef u64 opengl_state_id_t;



typedef void* opengl_state_t;



typedef struct _OPENGL_INTERNAL_STATE{
	opengl_state_id_t state_id;
	opengl_driver_instance_t driver_instance;
	u16 driver_opengl_version;
	const glsl_backend_descriptor_t* glsl_backend_descriptor;
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



#endif
