#ifndef _OPENGL_PROTOCOL_H_
#define _OPENGL_PROTOCOL_H_ 1
#include <sys/types.h>



#define OPENGL_PROTOCOL_TYPE_CLEAR 0x01
#define OPENGL_PROTOCOL_TYPE_SET_VIEWPORT 0x02
#define OPENGL_PROTOCOL_TYPE_CREATE_SHADER 0x03
#define OPENGL_PROTOCOL_TYPE_USE_SHADER 0x04
#define OPENGL_PROTOCOL_TYPE_DRAW 0x05
#define OPENGL_PROTOCOL_TYPE_UPDATE_VERTEX_ARRAY 0x06
#define OPENGL_PROTOCOL_TYPE_UPDATE_BUFFER 0x07
#define OPENGL_PROTOCOL_TYPE_SET_BUFFERS 0x08
#define OPENGL_PROTOCOL_TYPE_FLUSH 0x09
#define OPENGL_PROTOCOL_TYPE_UPDATE_TEXTURE 0x0a

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

#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_NONE 0
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA32F 1
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA32I 2
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA32UI 3
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16 4
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16F 5
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16I 6
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16UI 7
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA8 8
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA8UI 9
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_SRGB8_ALPHA8 10
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB10_A2 11
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB10_A2UI 12
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R11F_G11F_B10F 13
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG32F 14
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG32I 15
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG32UI 16
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG16 17
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG16F 18
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16I 19
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16UI 20
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8 21
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8I 22
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8UI 23
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R32F 24
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R32I 25
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R32UI 26
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16F 27
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16I 28
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16UI 29
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8 30
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8I 31
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8UI 32
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16_SNORM 33
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA8_SNORM 34
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB32F 35
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB32I 36
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB32UI 37
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16_SNORM 38
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16F 39
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16 40
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8_SNORM 41
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8 42
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8I 43
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8UI 44
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_SRGB8 45
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB9_E5 46
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG16_SNORM 47
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8_SNORM 48
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_RG_RGTC2 49
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_SIGNED_RG_RGTC2 50
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16_SNORM 51
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8_SNORM 52
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_RED_RGTC1 53
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_SIGNED_RED_RGTC1 54
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH_COMPONENT32F 55
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH_COMPONENT24 56
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH_COMPONENT16 57
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH32F_STENCIL8 58
#define OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8 59



typedef struct SYS_PACKED _OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT{
	u8 index;
	u8 size;
	u8 type;
	u8 stride;
	u8 divisor;
	u8 offset;
	_Bool require_normalization;
	u8 _padding;
} opengl_protocol_vertex_array_element_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_VERTEX_BUFFER_CONFIG{
	u64 driver_handle;
	u32 offset;
	u32 stride;
} opengl_protocol_vertex_buffer_config_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_HEADER{
	u32 _data[0];
	u8 type;
	u8 ret_code;
	u16 length;
} opengl_protocol_header_t;



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
	u32 _padding;
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
	u32 start;
	u32 count;
	u32 indexed;
	u32 instance_count;
	u32 index_bias;
	u32 start_instance;
	u32 pritmitive_restart_index;
	u32 min_index;
	u32 max_index;
} opengl_protocol_draw_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_UPDATE_VERTEX_ARRAY{
	opengl_protocol_header_t header;
	u32 count;
	u64 driver_handle;
	opengl_protocol_vertex_array_element_t elements[32];
} opengl_protocol_update_vertex_array_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_UPDATE_BUFFER{
	opengl_protocol_header_t header;
	u32 storage_type;
	u64 driver_handle;
	u32 offset;
	u32 size;
	const void* data;
} opengl_protocol_update_buffer_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_SET_BUFFERS{
	opengl_protocol_header_t header;
	u64 index_buffer_driver_handle;
	u32 index_buffer_index_width;
	u32 index_buffer_offset;
	const void* uniform_buffer_data;
	u32 uniform_buffer_size;
	u32 vertex_buffer_count;
	opengl_protocol_vertex_buffer_config_t vertex_buffers[32];
} opengl_protocol_set_buffers_t;



typedef struct SYS_PACKED _OPENGL_PROTOCOL_UPDATE_TEXTURE{
	opengl_protocol_header_t header;
	u32 format;
	u64 driver_handle;
	u32 x_offset;
	u32 y_offset;
	u32 z_offset;
	u32 width;
	u32 height;
	u32 depth;
	u32 level;
	const void* data;
} opengl_protocol_update_texture_t;



#endif
