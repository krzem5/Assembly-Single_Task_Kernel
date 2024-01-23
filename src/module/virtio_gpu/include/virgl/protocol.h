#ifndef _VIRGL_PROTOCOL_H_
#define _VIRGL_PROTOCOL_H_ 1
#include <kernel/types.h>



#define _VIRGL_PROTOCOL_COMMAND_HEADER(command,type,length) ((command)|((type)<<8)|((length)<<16))

#define VIRGL_PROTOCOL_COMMAND_NOP _VIRGL_PROTOCOL_COMMAND_HEADER(0,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT _VIRGL_PROTOCOL_COMMAND_HEADER(1,/*type*/,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_BLEND _VIRGL_PROTOCOL_COMMAND_HEADER(1,1,11)
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_RASTERIZER _VIRGL_PROTOCOL_COMMAND_HEADER(1,2,9)
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_DSA _VIRGL_PROTOCOL_COMMAND_HEADER(1,3,5)
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SHADER(length) _VIRGL_PROTOCOL_COMMAND_HEADER(1,4,5+(length))
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_VERTEX_ELEMENTS(count) _VIRGL_PROTOCOL_COMMAND_HEADER(1,5,1+((count)<<2))
#define VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SURFACE _VIRGL_PROTOCOL_COMMAND_HEADER(1,8,5)
#define VIRGL_PROTOCOL_COMMAND_BIND_OBJECT _VIRGL_PROTOCOL_COMMAND_HEADER(2,/*type*/,1)
#define VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_BLEND _VIRGL_PROTOCOL_COMMAND_HEADER(2,1,1)
#define VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_RASTERIZER _VIRGL_PROTOCOL_COMMAND_HEADER(2,2,1)
#define VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_DSA _VIRGL_PROTOCOL_COMMAND_HEADER(2,3,1)
#define VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_VERTEX_ELEMENTS _VIRGL_PROTOCOL_COMMAND_HEADER(2,5,1)
#define VIRGL_PROTOCOL_COMMAND_DESTROY_OBJECT _VIRGL_PROTOCOL_COMMAND_HEADER(3,/*type*/,1)
#define VIRGL_PROTOCOL_COMMAND_DESTROY_OBJECT_VERTEX_ELEMENTS _VIRGL_PROTOCOL_COMMAND_HEADER(3,5,1)
#define VIRGL_PROTOCOL_COMMAND_SET_VIEWPORT_STATE _VIRGL_PROTOCOL_COMMAND_HEADER(4,0,7)
#define VIRGL_PROTOCOL_COMMAND_SET_FRAMEBUFFER_STATE _VIRGL_PROTOCOL_COMMAND_HEADER(5,0,3)
#define VIRGL_PROTOCOL_COMMAND_SET_VERTEX_BUFFERS(count) _VIRGL_PROTOCOL_COMMAND_HEADER(6,0,(count)*3)
#define VIRGL_PROTOCOL_COMMAND_CLEAR _VIRGL_PROTOCOL_COMMAND_HEADER(7,0,8)
#define VIRGL_PROTOCOL_COMMAND_DRAW_VBO _VIRGL_PROTOCOL_COMMAND_HEADER(8,0,12)
#define VIRGL_PROTOCOL_COMMAND_RESOURCE_INLINE_WRITE _VIRGL_PROTOCOL_COMMAND_HEADER(9,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_SAMPLER_VIEWS _VIRGL_PROTOCOL_COMMAND_HEADER(10,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_INDEX_BUFFER _VIRGL_PROTOCOL_COMMAND_HEADER(11,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_CONSTANT_BUFFER _VIRGL_PROTOCOL_COMMAND_HEADER(12,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_STENCIL_REF _VIRGL_PROTOCOL_COMMAND_HEADER(13,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_BLEND_COLOR _VIRGL_PROTOCOL_COMMAND_HEADER(14,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_SCISSOR_STATE _VIRGL_PROTOCOL_COMMAND_HEADER(15,0,33)
#define VIRGL_PROTOCOL_COMMAND_BLIT _VIRGL_PROTOCOL_COMMAND_HEADER(16,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_RESOURCE_COPY_REGION _VIRGL_PROTOCOL_COMMAND_HEADER(17,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_BIND_SAMPLER_STATES _VIRGL_PROTOCOL_COMMAND_HEADER(18,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_BEGIN_QUERY _VIRGL_PROTOCOL_COMMAND_HEADER(19,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_END_QUERY _VIRGL_PROTOCOL_COMMAND_HEADER(20,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_GET_QUERY_RESULT _VIRGL_PROTOCOL_COMMAND_HEADER(21,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_POLYGON_STIPPLE _VIRGL_PROTOCOL_COMMAND_HEADER(22,0,32)
#define VIRGL_PROTOCOL_COMMAND_SET_CLIP_STATE _VIRGL_PROTOCOL_COMMAND_HEADER(23,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_SAMPLE_MASK _VIRGL_PROTOCOL_COMMAND_HEADER(24,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_STREAMOUT_TARGETS _VIRGL_PROTOCOL_COMMAND_HEADER(25,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_RENDER_CONDITION _VIRGL_PROTOCOL_COMMAND_HEADER(26,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_UNIFORM_BUFFER _VIRGL_PROTOCOL_COMMAND_HEADER(27,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_SUB_CTX _VIRGL_PROTOCOL_COMMAND_HEADER(28,0,1)
#define VIRGL_PROTOCOL_COMMAND_CREATE_SUB_CTX _VIRGL_PROTOCOL_COMMAND_HEADER(29,0,1)
#define VIRGL_PROTOCOL_COMMAND_DESTROY_SUB_CTX _VIRGL_PROTOCOL_COMMAND_HEADER(30,0,1)
#define VIRGL_PROTOCOL_COMMAND_BIND_SHADER _VIRGL_PROTOCOL_COMMAND_HEADER(31,0,2)
#define VIRGL_PROTOCOL_COMMAND_SET_TESS_STATE _VIRGL_PROTOCOL_COMMAND_HEADER(32,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_MIN_SAMPLES _VIRGL_PROTOCOL_COMMAND_HEADER(33,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_SHADER_BUFFERS _VIRGL_PROTOCOL_COMMAND_HEADER(34,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_SHADER_IMAGES _VIRGL_PROTOCOL_COMMAND_HEADER(35,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_MEMORY_BARRIER _VIRGL_PROTOCOL_COMMAND_HEADER(36,0,1)
#define VIRGL_PROTOCOL_COMMAND_LAUNCH_GRID _VIRGL_PROTOCOL_COMMAND_HEADER(37,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_FRAMEBUFFER_STATE_NO_ATTACH _VIRGL_PROTOCOL_COMMAND_HEADER(38,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_TEXTURE_BARRIER _VIRGL_PROTOCOL_COMMAND_HEADER(39,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_ATOMIC_BUFFERS _VIRGL_PROTOCOL_COMMAND_HEADER(40,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_DEBUG_FLAGS _VIRGL_PROTOCOL_COMMAND_HEADER(41,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_GET_QUERY_RESULT_QBO _VIRGL_PROTOCOL_COMMAND_HEADER(42,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_TRANSFER3D _VIRGL_PROTOCOL_COMMAND_HEADER(43,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_END_TRANSFERS _VIRGL_PROTOCOL_COMMAND_HEADER(44,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_COPY_TRANSFER3D _VIRGL_PROTOCOL_COMMAND_HEADER(45,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SET_TWEAKS _VIRGL_PROTOCOL_COMMAND_HEADER(46,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_CLEAR_TEXTURE _VIRGL_PROTOCOL_COMMAND_HEADER(47,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_PIPE_RESOURCE_CREATE _VIRGL_PROTOCOL_COMMAND_HEADER(48,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_PIPE_RESOURCE_SET_TYPE _VIRGL_PROTOCOL_COMMAND_HEADER(49,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_GET_MEMORY_INFO _VIRGL_PROTOCOL_COMMAND_HEADER(50,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_SEND_STRING_MARKER _VIRGL_PROTOCOL_COMMAND_HEADER(51,0,/*length*/0)
#define VIRGL_PROTOCOL_COMMAND_LINK_SHADER _VIRGL_PROTOCOL_COMMAND_HEADER(52,0,6)
#define VIRGL_PROTOCOL_COMMAND_CLEAR_SURFACE _VIRGL_PROTOCOL_COMMAND_HEADER(62,0,/*length*/0)

#define VIRGL_PROTOCOL_CLEAR_FLAG_COLOR 0x03fc
#define VIRGL_PROTOCOL_CLEAR_FLAG_DEPTH 0x0001
#define VIRGL_PROTOCOL_CLEAR_FLAG_STENCIL 0x0002

#define VIRGL_PROTOCOL_BIND_FLAG_DEPTH_STENCIL 0x00000001
#define VIRGL_PROTOCOL_BIND_FLAG_RENDER_TARGET 0x00000002
#define VIRGL_PROTOCOL_BIND_FLAG_SAMPLER_VIEW 0x00000008
#define VIRGL_PROTOCOL_BIND_FLAG_VERTEX_BUFFER 0x00000010
#define VIRGL_PROTOCOL_BIND_FLAG_GLOBAL 0x00040000

#define VIRGL_TARGET_BUFFER 0
#define VIRGL_TARGET_TEXTURE_2D 2

#define VIRGL_FORMAT_B8G8R8A8_UNORM 1
#define VIRGL_FORMAT_B8G8R8X8_UNORM 2
#define VIRGL_FORMAT_S8_UINT_Z24_UNORM 20
#define VIRGL_FORMAT_R32_FLOAT 28
#define VIRGL_FORMAT_R32G32_FLOAT 29
#define VIRGL_FORMAT_R8_UNORM 64

#define VIRGL_SHADER_VERTEX 0
#define VIRGL_SHADER_FRAGMENT 1

#define VIRGL_PRIMITIVE_POINTS 0
#define VIRGL_PRIMITIVE_LINES 1
#define VIRGL_PRIMITIVE_LINE_LOOP 2
#define VIRGL_PRIMITIVE_LINE_STRIP 3
#define VIRGL_PRIMITIVE_TRIANGLES 4
#define VIRGL_PRIMITIVE_TRIANGLE_STRIP 5
#define VIRGL_PRIMITIVE_TRIANGLE_FAN 6
#define VIRGL_PRIMITIVE_LINES_ADJACENCY 10
#define VIRGL_PRIMITIVE_LINE_STRIP_ADJACENCY 11
#define VIRGL_PRIMITIVE_TRIANGLES_ADJACENCY 12
#define VIRGL_PRIMITIVE_TRIANGLE_STRIP_ADJACENCY 13



#endif
