#ifndef _OPENGL_COMMAND_BUFFER_H_
#define _OPENGL_COMMAND_BUFFER_H_ 1
#include <opengl/protocol.h>
#include <sys/types.h>



#define OPENGL_COMMAND_BUFFER_SIZE 0x100000



void opengl_command_buffer_init(void);



void opengl_command_buffer_set_lock(_Bool lock);



void opengl_command_buffer_ensure_space(u32 length);



const opengl_protocol_header_t* opengl_command_buffer_push(const opengl_protocol_header_t* header);



void opengl_command_buffer_flush(void);



#endif
