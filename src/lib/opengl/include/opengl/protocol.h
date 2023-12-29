#ifndef _OPENGL_PROTOCOL_H_
#define _OPENGL_PROTOCOL_H_ 1
#include <sys/types.h>



typedef struct SYS_PACKED _OPENGL_PROTOCOL_HEADER{
	u8 command;
	u8 length;
} opengl_protocol_header_t;



#endif
