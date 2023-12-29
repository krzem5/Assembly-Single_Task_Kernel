#ifndef _OPENGL_PROTOCOL_H_
#define _OPENGL_PROTOCOL_H_ 1
#include <kernel/types.h>



#define OPENGL_PROTOCOL_TYPE_CREATE_RESOURCE 0x01
#define OPENGL_PROTOCOL_TYPE_DELETE_RESOURCE 0x02
#define OPENGL_PROTOCOL_TYPE_CLEAR 0x03
#define OPENGL_PROTOCOL_TYPE_SET_VIEWPORT 0x04

#define OPENGL_PROTOCOL_CLEAR_FLAG_COLOR 0x00000001
#define OPENGL_PROTOCOL_CLEAR_FLAG_DEPTH 0x00000002
#define OPENGL_PROTOCOL_CLEAR_FLAG_STENCIL 0x00000004



typedef union KERNEL_PACKED _OPENGL_PROTOCOL_FLOAT{
	float value;
	u32 raw_value;
} opengl_protocol_float_t;



typedef union KERNEL_PACKED _OPENGL_PROTOCOL_DOUBLE{
	double value;
	u64 raw_value;
} opengl_protocol_double_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_HEADER{
	u32 _data[0];
	u8 type;
	u8 length;
	u16 ret_code;
} opengl_protocol_header_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_CREATE_RESOURCE{
	opengl_protocol_header_t header;
	u32 sys_handle;
	u8 type;
	u8 usage;
	u16 format;
	u16 bind;
	u32 width;
	u32 height;
	u32 depth;
} opengl_protocol_create_resource_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_DELETE_RESOURCE{
	opengl_protocol_header_t header;
	u32 sys_handle;
} opengl_protocol_delete_resource_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_CLEAR{
	opengl_protocol_header_t header;
	u32 flags;
	opengl_protocol_float_t color[4];
	opengl_protocol_double_t depth;
	s32 stencil;
} opengl_protocol_clear_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_SET_VIEWPORT{
	opengl_protocol_header_t header;
	opengl_protocol_float_t tx;
	opengl_protocol_float_t ty;
	opengl_protocol_float_t sx;
	opengl_protocol_float_t sy;
} opengl_protocol_set_viewport_t;



#endif
