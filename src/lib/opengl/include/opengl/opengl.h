#ifndef _OPENGL_OPENGL_H_
#define _OPENGL_OPENGL_H_ 1
#include <sys/types.h>
#include <ui/display.h>



typedef void* opengl_state_t;



void opengl_init(void);



opengl_state_t opengl_create_state(u16 min_version);



_Bool opengl_set_state(opengl_state_t state);



_Bool opengl_set_state_framebuffer(opengl_state_t state,ui_framebuffer_handle_t framebuffer,ui_framebuffer_handle_t framebuffer2);



#endif
