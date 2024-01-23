#ifndef _OPENGL_SYSCALLS_H_
#define _OPENGL_SYSCALLS_H_ 1
#include <opengl/_internal/state.h>
#include <opengl/opengl.h>
#include <sys/types.h>
#include <ui/display.h>



_Bool opengl_syscalls_init(void);



opengl_driver_instance_t opengl_syscall_get_driver_instance(u16 min_version);



_Bool opengl_syscall_get_driver_instance_data(opengl_driver_instance_t instance,opengl_driver_instance_data_t* out);



opengl_state_id_t opengl_syscall_create_state(opengl_driver_instance_t instance);



_Bool opengl_syscall_delete_state(opengl_state_id_t state);



_Bool opengl_syscall_set_state(opengl_state_id_t state);



_Bool opengl_syscall_set_state_framebuffer(opengl_state_id_t state,ui_framebuffer_handle_t framebuffer,ui_framebuffer_handle_t framebuffer2);



void opengl_syscall_flush_command_buffer(void* buffer,u32 buffer_size);



#endif
