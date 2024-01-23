#ifndef _OPENGL_PROTOCOL_H_
#define _OPENGL_PROTOCOL_H_ 1
#include <kernel/types.h>



#define OPENGL_PROTOCOL_TYPE_CLEAR 0x01
#define OPENGL_PROTOCOL_TYPE_SET_VIEWPORT 0x02
#define OPENGL_PROTOCOL_TYPE_CREATE_SHADER 0x03
#define OPENGL_PROTOCOL_TYPE_USE_SHADER 0x04
#define OPENGL_PROTOCOL_TYPE_DRAW 0x05
#define OPENGL_PROTOCOL_TYPE_UPDATE_VERTEX_ARRAY 0x06
#define OPENGL_PROTOCOL_TYPE_UPDATE_BUFFER 0x07

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

#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_SIZE_BGRA 0xff

#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_BYTE 0
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_BYTE 1
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_SHORT 2
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_SHORT 3
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT 4
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT 5
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_HALF_FLOAT 6
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_FLOAT 7
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_DOUBLE 8
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT_2_10_10_10_REV 9
#define OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT_2_10_10_10_REV 10

#define OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_NO_CHANGE 0
#define OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_STREAM 1
#define OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_STATIC 2
#define OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_DYNAMIC 3



typedef union KERNEL_PACKED _OPENGL_PROTOCOL_FLOAT{
	float value;
	u32 raw_value;
} opengl_protocol_float_t;



typedef union KERNEL_PACKED _OPENGL_PROTOCOL_DOUBLE{
	double value;
	u64 raw_value;
} opengl_protocol_double_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT{
	u8 index;
	u8 size;
	u8 type;
	u8 stride;
	u8 divisor;
	u8 offset;
	_Bool require_normalization;
	u8 _padding;
} opengl_protocol_vertex_array_element_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_HEADER{
	u32 _data[0];
	u8 type;
	u8 ret_code;
	u16 length;
} opengl_protocol_header_t;



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



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_CREATE_SHADER{
	opengl_protocol_header_t header;
	u32 _padding;
	u64 driver_handle;
	u32 format;
	u32 vertex_shader_size;
	u32 fragment_shader_size;
	const u8* vertex_shader_data;
	const u8* fragment_shader_data;
} opengl_protocol_create_shader_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_USE_SHADER{
	opengl_protocol_header_t header;
	u64 driver_handle;
} opengl_protocol_use_shader_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_DRAW{
	opengl_protocol_header_t header;
	u32 mode;
	u32 first;
	u32 count;
	u32 instance_count;
} opengl_protocol_draw_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_UPDATE_VERTEX_ARRAY{
	opengl_protocol_header_t header;
	u32 count;
	u64 driver_handle;
	opengl_protocol_vertex_array_element_t elements[32];
} opengl_protocol_update_vertex_array_t;



typedef struct KERNEL_PACKED _OPENGL_PROTOCOL_UPDATE_BUFFER{
	opengl_protocol_header_t header;
	u32 storage_type;
	u64 driver_handle;
	u32 offset;
	u32 size;
	const void* data;
} opengl_protocol_update_buffer_t;



#endif
