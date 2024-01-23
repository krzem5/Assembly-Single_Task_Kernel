#ifndef _OPENGL_PROTOCOL_H_
#define _OPENGL_PROTOCOL_H_ 1
#include <sys/types.h>



#define OPENGL_PROTOCOL_TYPE_CREATE_RESOURCE 0x01
#define OPENGL_PROTOCOL_TYPE_DELETE_RESOURCE 0x02
#define OPENGL_PROTOCOL_TYPE_CLEAR 0x03
#define OPENGL_PROTOCOL_TYPE_SET_VIEWPORT 0x04
#define OPENGL_PROTOCOL_TYPE_CREATE_SHADER 0x05
#define OPENGL_PROTOCOL_TYPE_USE_SHADER 0x06
#define OPENGL_PROTOCOL_TYPE_DRAW 0x07

#define OPENGL_PROTOCOL_CLEAR_FLAG_COLOR 0x00000001
#define OPENGL_PROTOCOL_CLEAR_FLAG_DEPTH 0x00000002
#define OPENGL_PROTOCOL_CLEAR_FLAG_STENCIL 0x00000004

#define OPENGL_PROTOCOL_SHADER_FORMAT_TGSI 0x0001

#define OPENGL_PROTOCOL_DRAW_MODE_POINTS 0
#define OPENGL_PROTOCOL_DRAW_MODE_LINES 1
#define OPENGL_PROTOCOL_DRAW_MODE_LINE_LOOP 2
#define OPENGL_PROTOCOL_DRAW_MODE_LINE_STRIP 3
#define OPENGL_PROTOCOL_DRAW_MODE_TRIANGLES 4
#define OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_STRIP 5
#define OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_FAN 6
#define OPENGL_PROTOCOL_DRAW_MODE_LINES_ADJACENCY 7
#define OPENGL_PROTOCOL_DRAW_MODE_LINE_STRIP_ADJACENCY 8
#define OPENGL_PROTOCOL_DRAW_MODE_TRIANGLES_ADJACENCY 9
#define OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY 10



typedef struct SYS_PACKED _OPENGL_PROTOCOL_HEADER{
	u32 _data[0];
	u8 type;
	u8 length;
	u16 ret_code;
} opengl_protocol_header_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_CREATE_RESOURCE{
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



typedef struct SYS_PACKED _OPENGL_PROTOCOL_DELETE_RESOURCE{
	opengl_protocol_header_t header;
	u32 sys_handle;
} opengl_protocol_delete_resource_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_CLEAR{
	opengl_protocol_header_t header;
	u32 flags;
	float color[4];
	double depth;
	s32 stencil;
} opengl_protocol_clear_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_SET_VIEWPORT{
	opengl_protocol_header_t header;
	float tx;
	float ty;
	float sx;
	float sy;
} opengl_protocol_set_viewport_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_CREATE_SHADER{
	opengl_protocol_header_t header;
	u64 driver_handle;
	u32 format;
	u32 vertex_shader_size;
	u32 fragment_shader_size;
	const u8* vertex_shader_data;
	const u8* fragment_shader_data;
} opengl_protocol_create_shader_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_USE_SHADER{
	opengl_protocol_header_t header;
	u64 driver_handle;
} opengl_protocol_use_shader_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_DRAW{
	opengl_protocol_header_t header;
	u32 mode;
	u32 first;
	u32 count;
	u32 instance_count;
} opengl_protocol_draw_t;



#endif
