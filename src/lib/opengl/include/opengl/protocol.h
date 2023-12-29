#ifndef _OPENGL_PROTOCOL_H_
#define _OPENGL_PROTOCOL_H_ 1
#include <sys/types.h>



#define OPENGL_PROTOCOL_TYPE_CREATE_RESOURCE 0x01
#define OPENGL_PROTOCOL_TYPE_DELETE_RESOURCE 0x02



typedef struct SYS_PACKED _OPENGL_PROTOCOL_HEADER{
	u8 type;
	u8 length;
} opengl_protocol_header_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_CREATE_RESOURCE{
	opengl_protocol_header_t header;
	u16 format;
	u32 sys_handle;
	u32 width;
	u32 height;
	u32 depth;
	u32 _padding;
} opengl_protocol_create_resource_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_DELETE_RESOURCE{
	opengl_protocol_header_t header;
	u16 _padding;
	u32 sys_handle;
} opengl_protocol_delete_resource_t;



#endif
