#include <GL/gl.h>
#include <opengl/_internal/state.h>
#include <opengl/command_buffer.h>
#include <opengl/config.h>
#include <opengl/protocol.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



#define OPENGL_PARAMETER_NO_INDEX 0xffffffffffffffffull

#define OPENGL_PARAMETER_TYPE_BOOL 0
#define OPENGL_PARAMETER_TYPE_INT 1
#define OPENGL_PARAMETER_TYPE_INT64 2
#define OPENGL_PARAMETER_TYPE_FLOAT 3
#define OPENGL_PARAMETER_TYPE_DOUBLE 4
#define OPENGL_PARAMETER_TYPE_COLOR_OR_NORMAL 5

#define OPENGL_PARAMETER_RETURN_TYPE_BOOL 0
#define OPENGL_PARAMETER_RETURN_TYPE_INT 1
#define OPENGL_PARAMETER_RETURN_TYPE_INT64 2
#define OPENGL_PARAMETER_RETURN_TYPE_FLOAT 3
#define OPENGL_PARAMETER_RETURN_TYPE_DOUBLE 4



static opengl_internal_state_t* _gl_internal_state=NULL;

static const GLuint _gl_max_vertex_attribs=16;



static void* _alloc_handle(opengl_handle_type_t type,u32 size){
	u32 index=0;
	for (;index<_gl_internal_state->handle_count&&_gl_internal_state->handles[index];index++);
	if (index==_gl_internal_state->handle_count){
		_gl_internal_state->handle_count++;
		_gl_internal_state->handles=sys_heap_realloc(NULL,_gl_internal_state->handles,_gl_internal_state->handle_count*sizeof(opengl_handle_header_t*));
	}
	opengl_handle_header_t* out=sys_heap_alloc(NULL,size);
	out->type=type;
	out->index=index;
	_gl_internal_state->handles[index]=out;
	return out;
}



static void* _get_handle(GLuint handle,opengl_handle_type_t type){
	if (handle>=_gl_internal_state->handle_count||!_gl_internal_state->handles[handle]){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return NULL;
	}
	if (_gl_internal_state->handles[handle]->type!=type){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return NULL;
	}
	return _gl_internal_state->handles[handle];
}



static void _shader_state_remove_sources(opengl_shader_state_t* state){
	if (!state->source_count){
		return;
	}
	for (u32 i=0;i<state->source_count;i++){
		sys_heap_dealloc(NULL,(state->sources+i)->data);
	}
	sys_heap_dealloc(NULL,state->sources);
	state->source_count=0;
	state->sources=NULL;
}



static void _gl_get_parameter(GLenum param,u64 index,void* out,u32 out_type){
	GLuint local_value;
	u32 type;
	u32 length;
	const void* values;
	switch (param){
		case GL_ACTIVE_TEXTURE:
			local_value=_gl_internal_state->gl_active_texture+GL_TEXTURE0;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_ALIASED_LINE_WIDTH_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_ALIASED_LINE_WIDTH_RANGE\x1b[0m\n");
			return;
		case GL_SMOOTH_LINE_WIDTH_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SMOOTH_LINE_WIDTH_RANGE\x1b[0m\n");
			return;
		case GL_SMOOTH_LINE_WIDTH_GRANULARITY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SMOOTH_LINE_WIDTH_GRANULARITY\x1b[0m\n");
			return;
		case GL_ARRAY_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_ARRAY_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_BLEND:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND\x1b[0m\n");
			return;
		case GL_BLEND_COLOR:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_COLOR\x1b[0m\n");
			return;
		case GL_BLEND_DST_ALPHA:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_DST_ALPHA\x1b[0m\n");
			return;
		case GL_BLEND_DST_RGB:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_DST_RGB\x1b[0m\n");
			return;
		case GL_BLEND_EQUATION_RGB:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_EQUATION_RGB\x1b[0m\n");
			return;
		case GL_BLEND_EQUATION_ALPHA:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_EQUATION_ALPHA\x1b[0m\n");
			return;
		case GL_BLEND_SRC_ALPHA:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_SRC_ALPHA\x1b[0m\n");
			return;
		case GL_BLEND_SRC_RGB:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_BLEND_SRC_RGB\x1b[0m\n");
			return;
		case GL_COLOR_CLEAR_VALUE:
			type=OPENGL_PARAMETER_TYPE_COLOR_OR_NORMAL;
			length=4;
			values=_gl_internal_state->gl_clear_color_value;
			break;
		case GL_COLOR_LOGIC_OP:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_COLOR_LOGIC_OP\x1b[0m\n");
			return;
		case GL_COLOR_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_COLOR_WRITEMASK\x1b[0m\n");
			return;
		case GL_COMPRESSED_TEXTURE_FORMATS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_COMPRESSED_TEXTURE_FORMATS\x1b[0m\n");
			return;
		case GL_CULL_FACE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_CULL_FACE\x1b[0m\n");
			return;
		case GL_CURRENT_PROGRAM:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_CURRENT_PROGRAM\x1b[0m\n");
			return;
		case GL_DEPTH_CLEAR_VALUE:
			type=OPENGL_PARAMETER_TYPE_DOUBLE;
			length=1;
			values=&_gl_internal_state->gl_clear_depth_value;
			break;
		case GL_DEPTH_FUNC:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DEPTH_FUNC\x1b[0m\n");
			return;
		case GL_DEPTH_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DEPTH_RANGE\x1b[0m\n");
			return;
		case GL_DEPTH_TEST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DEPTH_TEST\x1b[0m\n");
			return;
		case GL_DEPTH_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DEPTH_WRITEMASK\x1b[0m\n");
			return;
		case GL_DITHER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DITHER\x1b[0m\n");
			return;
		case GL_DOUBLEBUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DOUBLEBUFFER\x1b[0m\n");
			return;
		case GL_DRAW_BUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DRAW_BUFFER\x1b[0m\n");
			return;
		case GL_DRAW_BUFFER0:
		case GL_DRAW_BUFFER1:
		case GL_DRAW_BUFFER2:
		case GL_DRAW_BUFFER3:
		case GL_DRAW_BUFFER4:
		case GL_DRAW_BUFFER5:
		case GL_DRAW_BUFFER6:
		case GL_DRAW_BUFFER7:
		case GL_DRAW_BUFFER8:
		case GL_DRAW_BUFFER9:
		case GL_DRAW_BUFFER10:
		case GL_DRAW_BUFFER11:
		case GL_DRAW_BUFFER12:
		case GL_DRAW_BUFFER13:
		case GL_DRAW_BUFFER14:
		case GL_DRAW_BUFFER15:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DRAW_BUFFER%u\x1b[0m\n",param-GL_DRAW_BUFFER0);
			return;
		case GL_DRAW_FRAMEBUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_DRAW_FRAMEBUFFER_BINDING\x1b[0m\n");
			return;
		case GL_READ_FRAMEBUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_READ_FRAMEBUFFER_BINDING\x1b[0m\n");
			return;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_ELEMENT_ARRAY_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_RENDERBUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_RENDERBUFFER_BINDING\x1b[0m\n");
			return;
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_FRAGMENT_SHADER_DERIVATIVE_HINT\x1b[0m\n");
			return;
		case GL_LINE_SMOOTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_LINE_SMOOTH\x1b[0m\n");
			return;
		case GL_LINE_SMOOTH_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_LINE_SMOOTH_HINT\x1b[0m\n");
			return;
		case GL_LINE_WIDTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_LINE_WIDTH\x1b[0m\n");
			return;
		case GL_LOGIC_OP_MODE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_LOGIC_OP_MODE\x1b[0m\n");
			return;
		case GL_MAX_3D_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_3D_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_CLIP_DISTANCES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_CLIP_DISTANCES\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
			local_value=OPENGL_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_COMBINED_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_CUBE_MAP_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_DRAW_BUFFERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_DRAW_BUFFERS\x1b[0m\n");
			return;
		case GL_MAX_DUAL_SOURCE_DRAW_BUFFERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_DUAL_SOURCE_DRAW_BUFFERS\x1b[0m\n");
			return;
		case GL_MAX_ELEMENTS_INDICES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_ELEMENTS_INDICES\x1b[0m\n");
			return;
		case GL_MAX_ELEMENTS_VERTICES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_ELEMENTS_VERTICES\x1b[0m\n");
			return;
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_FRAGMENT_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_FRAGMENT_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_FRAGMENT_INPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MIN_PROGRAM_TEXEL_OFFSET:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MIN_PROGRAM_TEXEL_OFFSET\x1b[0m\n");
			return;
		case GL_MAX_PROGRAM_TEXEL_OFFSET:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_PROGRAM_TEXEL_OFFSET\x1b[0m\n");
			return;
		case GL_MAX_RECTANGLE_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_RECTANGLE_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_IMAGE_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_TEXTURE_IMAGE_UNITS\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_LOD_BIAS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_TEXTURE_LOD_BIAS\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_RENDERBUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_RENDERBUFFER_SIZE\x1b[0m\n");
			return;
		case GL_MAX_ARRAY_TEXTURE_LAYERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_ARRAY_TEXTURE_LAYERS\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_BUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_TEXTURE_BUFFER_SIZE\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_ATTRIBS:
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&_gl_max_vertex_attribs;
			break;
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_VERTEX_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_VERTEX_OUTPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_GEOMETRY_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_SAMPLE_MASK_WORDS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_SAMPLE_MASK_WORDS\x1b[0m\n");
			return;
		case GL_MAX_COLOR_TEXTURE_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_COLOR_TEXTURE_SAMPLES\x1b[0m\n");
			return;
		case GL_MAX_DEPTH_TEXTURE_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_DEPTH_TEXTURE_SAMPLES\x1b[0m\n");
			return;
		case GL_MAX_INTEGER_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_INTEGER_SAMPLES\x1b[0m\n");
			return;
		case GL_MAX_SERVER_WAIT_TIMEOUT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_SERVER_WAIT_TIMEOUT\x1b[0m\n");
			return;
		case GL_MAX_UNIFORM_BUFFER_BINDINGS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_UNIFORM_BUFFER_BINDINGS\x1b[0m\n");
			return;
		case GL_MAX_UNIFORM_BLOCK_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_UNIFORM_BLOCK_SIZE\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_VERTEX_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_GEOMETRY_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_INPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_GEOMETRY_INPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_GEOMETRY_OUTPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_VIEWPORT_DIMS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_MAX_VIEWPORT_DIMS\x1b[0m\n");
			return;
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_NUM_COMPRESSED_TEXTURE_FORMATS\x1b[0m\n");
			return;
		case GL_PACK_ALIGNMENT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_ALIGNMENT\x1b[0m\n");
			return;
		case GL_PACK_IMAGE_HEIGHT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_IMAGE_HEIGHT\x1b[0m\n");
			return;
		case GL_PACK_LSB_FIRST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_LSB_FIRST\x1b[0m\n");
			return;
		case GL_PACK_ROW_LENGTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_ROW_LENGTH\x1b[0m\n");
			return;
		case GL_PACK_SKIP_IMAGES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_SKIP_IMAGES\x1b[0m\n");
			return;
		case GL_PACK_SKIP_PIXELS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_SKIP_PIXELS\x1b[0m\n");
			return;
		case GL_PACK_SKIP_ROWS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_SKIP_ROWS\x1b[0m\n");
			return;
		case GL_PACK_SWAP_BYTES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PACK_SWAP_BYTES\x1b[0m\n");
			return;
		case GL_PIXEL_PACK_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PIXEL_PACK_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PIXEL_UNPACK_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_POINT_FADE_THRESHOLD_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POINT_FADE_THRESHOLD_SIZE\x1b[0m\n");
			return;
		case GL_PRIMITIVE_RESTART_INDEX:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PRIMITIVE_RESTART_INDEX\x1b[0m\n");
			return;
		case GL_PROGRAM_POINT_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PROGRAM_POINT_SIZE\x1b[0m\n");
			return;
		case GL_PROVOKING_VERTEX:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_PROVOKING_VERTEX\x1b[0m\n");
			return;
		case GL_POINT_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POINT_SIZE\x1b[0m\n");
			return;
		case GL_POINT_SIZE_GRANULARITY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POINT_SIZE_GRANULARITY\x1b[0m\n");
			return;
		case GL_POINT_SIZE_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POINT_SIZE_RANGE\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_FACTOR:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_OFFSET_FACTOR\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_OFFSET_UNITS\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_FILL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_OFFSET_FILL\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_LINE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_OFFSET_LINE\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_POINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_OFFSET_POINT\x1b[0m\n");
			return;
		case GL_POLYGON_SMOOTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_SMOOTH\x1b[0m\n");
			return;
		case GL_POLYGON_SMOOTH_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_POLYGON_SMOOTH_HINT\x1b[0m\n");
			return;
		case GL_READ_BUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_READ_BUFFER\x1b[0m\n");
			return;
		case GL_SAMPLE_BUFFERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SAMPLE_BUFFERS\x1b[0m\n");
			return;
		case GL_SAMPLE_COVERAGE_VALUE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SAMPLE_COVERAGE_VALUE\x1b[0m\n");
			return;
		case GL_SAMPLE_COVERAGE_INVERT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SAMPLE_COVERAGE_INVERT\x1b[0m\n");
			return;
		case GL_SAMPLER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SAMPLER_BINDING\x1b[0m\n");
			return;
		case GL_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SAMPLES\x1b[0m\n");
			return;
		case GL_SCISSOR_BOX:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SCISSOR_BOX\x1b[0m\n");
			return;
		case GL_SCISSOR_TEST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SCISSOR_TEST\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_FUNC:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_FUNC\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_PASS_DEPTH_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_PASS_DEPTH_PASS\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_REF:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_REF\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_VALUE_MASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_VALUE_MASK\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_BACK_WRITEMASK\x1b[0m\n");
			return;
		case GL_STENCIL_CLEAR_VALUE:
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&_gl_internal_state->gl_clear_stencil_value;
			break;
		case GL_STENCIL_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_FUNC:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_FUNC\x1b[0m\n");
			return;
		case GL_STENCIL_PASS_DEPTH_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_PASS_DEPTH_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_PASS_DEPTH_PASS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_PASS_DEPTH_PASS\x1b[0m\n");
			return;
		case GL_STENCIL_REF:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_REF\x1b[0m\n");
			return;
		case GL_STENCIL_TEST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_TEST\x1b[0m\n");
			return;
		case GL_STENCIL_VALUE_MASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_VALUE_MASK\x1b[0m\n");
			return;
		case GL_STENCIL_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STENCIL_WRITEMASK\x1b[0m\n");
			return;
		case GL_STEREO:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_STEREO\x1b[0m\n");
			return;
		case GL_SUBPIXEL_BITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_SUBPIXEL_BITS\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_1D:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_1D\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_1D_ARRAY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_1D_ARRAY\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_2D\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D_ARRAY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_2D_ARRAY\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_2D_MULTISAMPLE\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_3D:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_3D\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_BUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_BUFFER\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_CUBE_MAP:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_CUBE_MAP\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_RECTANGLE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_BINDING_RECTANGLE\x1b[0m\n");
			return;
		case GL_TEXTURE_COMPRESSION_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TEXTURE_COMPRESSION_HINT\x1b[0m\n");
			return;
		case GL_TIMESTAMP:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TIMESTAMP\x1b[0m\n");
			return;
		case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TRANSFORM_FEEDBACK_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_TRANSFORM_FEEDBACK_BUFFER_START:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TRANSFORM_FEEDBACK_BUFFER_START\x1b[0m\n");
			return;
		case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_TRANSFORM_FEEDBACK_BUFFER_SIZE\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNIFORM_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_START:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNIFORM_BUFFER_START\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNIFORM_BUFFER_SIZE\x1b[0m\n");
			return;
		case GL_UNPACK_ALIGNMENT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_ALIGNMENT\x1b[0m\n");
			return;
		case GL_UNPACK_IMAGE_HEIGHT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_IMAGE_HEIGHT\x1b[0m\n");
			return;
		case GL_UNPACK_LSB_FIRST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_LSB_FIRST\x1b[0m\n");
			return;
		case GL_UNPACK_ROW_LENGTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_ROW_LENGTH\x1b[0m\n");
			return;
		case GL_UNPACK_SKIP_IMAGES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_SKIP_IMAGES\x1b[0m\n");
			return;
		case GL_UNPACK_SKIP_PIXELS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_SKIP_PIXELS\x1b[0m\n");
			return;
		case GL_UNPACK_SKIP_ROWS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_SKIP_ROWS\x1b[0m\n");
			return;
		case GL_UNPACK_SWAP_BYTES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_UNPACK_SWAP_BYTES\x1b[0m\n");
			return;
		case GL_NUM_EXTENSIONS:
			local_value=0;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_MAJOR_VERSION:
			local_value=_gl_internal_state->driver_opengl_version/100;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_MINOR_VERSION:
			local_value=(_gl_internal_state->driver_opengl_version/10)%10;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_CONTEXT_FLAGS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _gl_get_parameter.GL_CONTEXT_FLAGS\x1b[0m\n");
			return;
		case GL_VIEWPORT:
			type=OPENGL_PARAMETER_TYPE_INT;
			length=4;
			values=_gl_internal_state->gl_viewport;
			break;
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			return;
	}
	if (index!=OPENGL_PARAMETER_NO_INDEX&&length<=index){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	for (u32 i=(index==OPENGL_PARAMETER_NO_INDEX?0:index);i<(index==OPENGL_PARAMETER_NO_INDEX?length:index+1);i++){
		if (type==OPENGL_PARAMETER_TYPE_BOOL){
			GLboolean value=*(((const GLboolean*)values)+i);
			if (out_type==OPENGL_PARAMETER_RETURN_TYPE_BOOL){
				*(((GLboolean*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT){
				*(((GLint*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT64){
				*(((GLint64*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_FLOAT){
				*(((GLfloat*)out)+i)=value;
			}
			else{
				*(((GLdouble*)out)+i)=value;
			}
		}
		else if (type==OPENGL_PARAMETER_TYPE_INT){
			GLint value=*(((const GLint*)values)+i);
			if (out_type==OPENGL_PARAMETER_RETURN_TYPE_BOOL){
				*(((GLboolean*)out)+i)=(value?GL_TRUE:GL_FALSE);
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT){
				*(((GLint*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT64){
				*(((GLint64*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_FLOAT){
				*(((GLfloat*)out)+i)=value;
			}
			else{
				*(((GLdouble*)out)+i)=value;
			}
		}
		else if (type==OPENGL_PARAMETER_TYPE_INT64){
			GLint64 value=*(((const GLint64*)values)+i);
			if (out_type==OPENGL_PARAMETER_RETURN_TYPE_BOOL){
				*(((GLboolean*)out)+i)=(value?GL_TRUE:GL_FALSE);
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT){
				*(((GLint*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT64){
				*(((GLint64*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_FLOAT){
				*(((GLfloat*)out)+i)=value;
			}
			else{
				*(((GLdouble*)out)+i)=value;
			}
		}
		else if (type==OPENGL_PARAMETER_TYPE_FLOAT){
			GLfloat value=*(((const GLfloat*)values)+i);
			if (out_type==OPENGL_PARAMETER_RETURN_TYPE_BOOL){
				*(((GLboolean*)out)+i)=(value!=0.0f?GL_TRUE:GL_FALSE);
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT){
				*(((GLint*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT64){
				*(((GLint64*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_FLOAT){
				*(((GLfloat*)out)+i)=value;
			}
			else{
				*(((GLdouble*)out)+i)=value;
			}
		}
		else if (type==OPENGL_PARAMETER_TYPE_DOUBLE){
			GLdouble value=*(((const GLdouble*)values)+i);
			if (out_type==OPENGL_PARAMETER_RETURN_TYPE_BOOL){
				*(((GLboolean*)out)+i)=(value!=0.0?GL_TRUE:GL_FALSE);
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT){
				*(((GLint*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT64){
				*(((GLint64*)out)+i)=value;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_FLOAT){
				*(((GLfloat*)out)+i)=value;
			}
			else{
				*(((GLdouble*)out)+i)=value;
			}
		}
		else{
			GLfloat value=*(((const GLfloat*)values)+i);
			if (out_type==OPENGL_PARAMETER_RETURN_TYPE_BOOL){
				*(((GLboolean*)out)+i)=(value!=0.0f?GL_TRUE:GL_FALSE);
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT){
				*(((GLint*)out)+i)=(value+1)/2*0xffffffff-0x80000000;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_INT64){
				*(((GLint64*)out)+i)=(value+1)/2*0xffffffffffffffffll-0x8000000000000000ll;
			}
			else if (out_type==OPENGL_PARAMETER_RETURN_TYPE_FLOAT){
				*(((GLfloat*)out)+i)=value;
			}
			else{
				*(((GLdouble*)out)+i)=value;
			}
		}
	}
}



void _gl_set_internal_state(void* internal_state){
	_gl_internal_state=internal_state;
}



SYS_PUBLIC void glActiveTexture(GLenum texture){
	if (texture<GL_TEXTURE0||texture>=GL_TEXTURE0+OPENGL_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS){
		_gl_internal_state->gl_error=GL_INVALID_ENUM;
		return;
	}
	_gl_internal_state->gl_active_texture=texture-GL_TEXTURE0;
}



SYS_PUBLIC void glAttachShader(GLuint program,GLuint shader){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glAttachShader\x1b[0m\n");
}



SYS_PUBLIC void glBeginConditionalRender(GLuint id,GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBeginConditionalRender\x1b[0m\n");
}



SYS_PUBLIC void glBeginQuery(GLenum target,GLuint id){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBeginQuery\x1b[0m\n");
}



SYS_PUBLIC void glBeginTransformFeedback(GLenum primitiveMode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBeginTransformFeedback\x1b[0m\n");
}



SYS_PUBLIC void glBindAttribLocation(GLuint program,GLuint index,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindAttribLocation\x1b[0m\n");
}



SYS_PUBLIC void glBindBuffer(GLenum target,GLuint buffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindBuffer\x1b[0m\n");
}



SYS_PUBLIC void glBindBufferBase(GLenum target,GLuint index,GLuint buffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindBufferBase\x1b[0m\n");
}



SYS_PUBLIC void glBindBufferRange(GLenum target,GLuint index,GLuint buffer,GLintptr offset,GLsizeiptr size){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindBufferRange\x1b[0m\n");
}



SYS_PUBLIC void glBindFragDataLocation(GLuint program,GLuint color,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindFragDataLocation\x1b[0m\n");
}



SYS_PUBLIC void glBindFragDataLocationIndexed(GLuint program,GLuint colorNumber,GLuint index,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindFragDataLocationIndexed\x1b[0m\n");
}



SYS_PUBLIC void glBindFramebuffer(GLenum target,GLuint framebuffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindFramebuffer\x1b[0m\n");
}



SYS_PUBLIC void glBindRenderbuffer(GLenum target,GLuint renderbuffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindRenderbuffer\x1b[0m\n");
}



SYS_PUBLIC void glBindSampler(GLuint unit,GLuint sampler){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindSampler\x1b[0m\n");
}



SYS_PUBLIC void glBindTexture(GLenum target,GLuint texture){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindTexture\x1b[0m\n");
}



SYS_PUBLIC void glBindVertexArray(GLuint array){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindVertexArray\x1b[0m\n");
}



SYS_PUBLIC void glBlendColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBlendColor\x1b[0m\n");
}



SYS_PUBLIC void glBlendEquation(GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBlendEquation\x1b[0m\n");
}



SYS_PUBLIC void glBlendEquationSeparate(GLenum modeRGB,GLenum modeAlpha){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBlendEquationSeparate\x1b[0m\n");
}



SYS_PUBLIC void glBlendFunc(GLenum sfactor,GLenum dfactor){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBlendFunc\x1b[0m\n");
}



SYS_PUBLIC void glBlendFuncSeparate(GLenum sfactorRGB,GLenum dfactorRGB,GLenum sfactorAlpha,GLenum dfactorAlpha){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBlendFuncSeparate\x1b[0m\n");
}



SYS_PUBLIC void glBlitFramebuffer(GLint srcX0,GLint srcY0,GLint srcX1,GLint srcY1,GLint dstX0,GLint dstY0,GLint dstX1,GLint dstY1,GLbitfield mask,GLenum filter){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBlitFramebuffer\x1b[0m\n");
}



SYS_PUBLIC void glBufferData(GLenum target,GLsizeiptr size,const void* data,GLenum usage){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBufferData\x1b[0m\n");
}



SYS_PUBLIC void glBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBufferSubData\x1b[0m\n");
}



SYS_PUBLIC GLenum glCheckFramebufferStatus(GLenum target){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCheckFramebufferStatus\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glClampColor(GLenum target,GLenum clamp){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glClampColor\x1b[0m\n");
}



SYS_PUBLIC void glClear(GLbitfield mask){
	if (!mask){
		return;
	}
	if (mask&(~(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT))){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	u32 flags=0;
	if (mask&GL_COLOR_BUFFER_BIT){
		flags|=OPENGL_PROTOCOL_CLEAR_FLAG_COLOR;
	}
	if (mask&GL_DEPTH_BUFFER_BIT){
		flags|=OPENGL_PROTOCOL_CLEAR_FLAG_DEPTH;
	}
	if (mask&GL_STENCIL_BUFFER_BIT){
		flags|=OPENGL_PROTOCOL_CLEAR_FLAG_STENCIL;
	}
	opengl_protocol_clear_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_CLEAR,
		.header.length=sizeof(opengl_protocol_clear_t),
		.flags=flags,
		.color[0]=_gl_internal_state->gl_clear_color_value[0],
		.color[1]=_gl_internal_state->gl_clear_color_value[1],
		.color[2]=_gl_internal_state->gl_clear_color_value[2],
		.color[3]=_gl_internal_state->gl_clear_color_value[3],
		.depth=_gl_internal_state->gl_clear_depth_value,
		.stencil=_gl_internal_state->gl_clear_stencil_value,
	};
	opengl_command_buffer_push_single(&(command.header));
}



SYS_PUBLIC void glClearBufferfi(GLenum buffer,GLint drawbuffer,GLfloat depth,GLint stencil){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glClearBufferfi\x1b[0m\n");
}



SYS_PUBLIC void glClearBufferfv(GLenum buffer,GLint drawbuffer,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glClearBufferfv\x1b[0m\n");
}



SYS_PUBLIC void glClearBufferiv(GLenum buffer,GLint drawbuffer,const GLint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glClearBufferiv\x1b[0m\n");
}



SYS_PUBLIC void glClearBufferuiv(GLenum buffer,GLint drawbuffer,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glClearBufferuiv\x1b[0m\n");
}



SYS_PUBLIC void glClearColor(GLfloat red,GLfloat green,GLfloat blue,GLfloat alpha){
	_gl_internal_state->gl_clear_color_value[0]=red;
	_gl_internal_state->gl_clear_color_value[1]=green;
	_gl_internal_state->gl_clear_color_value[2]=blue;
	_gl_internal_state->gl_clear_color_value[3]=alpha;
}



SYS_PUBLIC void glClearDepth(GLdouble depth){
	_gl_internal_state->gl_clear_depth_value=depth;
}



SYS_PUBLIC void glClearStencil(GLint s){
	_gl_internal_state->gl_clear_stencil_value=s;
}



SYS_PUBLIC GLenum glClientWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glClientWaitSync\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glColorMask(GLboolean red,GLboolean green,GLboolean blue,GLboolean alpha){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glColorMask\x1b[0m\n");
}



SYS_PUBLIC void glColorMaski(GLuint index,GLboolean r,GLboolean g,GLboolean b,GLboolean a){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glColorMaski\x1b[0m\n");
}



SYS_PUBLIC void glCompileShader(GLuint shader){
	opengl_shader_state_t* state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER);
	if (!state){
		return;
	}
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompileShader\x1b[0m\n");
}



SYS_PUBLIC void glCompressedTexImage1D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLint border,GLsizei imageSize,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompressedTexImage1D\x1b[0m\n");
}



SYS_PUBLIC void glCompressedTexImage2D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLint border,GLsizei imageSize,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompressedTexImage2D\x1b[0m\n");
}



SYS_PUBLIC void glCompressedTexImage3D(GLenum target,GLint level,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLsizei imageSize,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompressedTexImage3D\x1b[0m\n");
}



SYS_PUBLIC void glCompressedTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLsizei imageSize,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompressedTexSubImage1D\x1b[0m\n");
}



SYS_PUBLIC void glCompressedTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLsizei imageSize,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompressedTexSubImage2D\x1b[0m\n");
}



SYS_PUBLIC void glCompressedTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLsizei imageSize,const void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCompressedTexSubImage3D\x1b[0m\n");
}



SYS_PUBLIC void glCopyBufferSubData(GLenum readTarget,GLenum writeTarget,GLintptr readOffset,GLintptr writeOffset,GLsizeiptr size){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCopyBufferSubData\x1b[0m\n");
}



SYS_PUBLIC void glCopyTexImage1D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLint border){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCopyTexImage1D\x1b[0m\n");
}



SYS_PUBLIC void glCopyTexImage2D(GLenum target,GLint level,GLenum internalformat,GLint x,GLint y,GLsizei width,GLsizei height,GLint border){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCopyTexImage2D\x1b[0m\n");
}



SYS_PUBLIC void glCopyTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLint x,GLint y,GLsizei width){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCopyTexSubImage1D\x1b[0m\n");
}



SYS_PUBLIC void glCopyTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint x,GLint y,GLsizei width,GLsizei height){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCopyTexSubImage2D\x1b[0m\n");
}



SYS_PUBLIC void glCopyTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLint x,GLint y,GLsizei width,GLsizei height){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCopyTexSubImage3D\x1b[0m\n");
}



SYS_PUBLIC GLuint glCreateProgram(void){
	opengl_program_state_t* state=_alloc_handle(OPENGL_HANDLE_TYPE_PROGRAM,sizeof(opengl_program_state_t));
	return state->header.index;
}



SYS_PUBLIC GLuint glCreateShader(GLenum type){
	if (type!=GL_VERTEX_SHADER&&type!=GL_FRAGMENT_SHADER){
		_gl_internal_state->gl_error=GL_INVALID_ENUM;
		return 0;
	}
	opengl_shader_state_t* state=_alloc_handle(OPENGL_HANDLE_TYPE_SHADER,sizeof(opengl_shader_state_t));
	state->type=type;
	state->source_count=0;
	state->sources=NULL;
	return state->header.index;
}



SYS_PUBLIC void glCullFace(GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glCullFace\x1b[0m\n");
}



SYS_PUBLIC void glDeleteBuffers(GLsizei n,const GLuint* buffers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteBuffers\x1b[0m\n");
}



SYS_PUBLIC void glDeleteFramebuffers(GLsizei n,const GLuint* framebuffers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteFramebuffers\x1b[0m\n");
}



SYS_PUBLIC void glDeleteProgram(GLuint program){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteProgram\x1b[0m\n");
}



SYS_PUBLIC void glDeleteQueries(GLsizei n,const GLuint* ids){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteQueries\x1b[0m\n");
}



SYS_PUBLIC void glDeleteRenderbuffers(GLsizei n,const GLuint* renderbuffers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteRenderbuffers\x1b[0m\n");
}



SYS_PUBLIC void glDeleteSamplers(GLsizei count,const GLuint* samplers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteSamplers\x1b[0m\n");
}



SYS_PUBLIC void glDeleteShader(GLuint shader){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteShader\x1b[0m\n");
}



SYS_PUBLIC void glDeleteSync(GLsync sync){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteSync\x1b[0m\n");
}



SYS_PUBLIC void glDeleteTextures(GLsizei n,const GLuint* textures){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteTextures\x1b[0m\n");
}



SYS_PUBLIC void glDeleteVertexArrays(GLsizei n,const GLuint* arrays){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDeleteVertexArrays\x1b[0m\n");
}



SYS_PUBLIC void glDepthFunc(GLenum func){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDepthFunc\x1b[0m\n");
}



SYS_PUBLIC void glDepthMask(GLboolean flag){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDepthMask\x1b[0m\n");
}



SYS_PUBLIC void glDepthRange(GLdouble n,GLdouble f){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDepthRange\x1b[0m\n");
}



SYS_PUBLIC void glDetachShader(GLuint program,GLuint shader){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDetachShader\x1b[0m\n");
}



SYS_PUBLIC void glDisable(GLenum cap){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDisable\x1b[0m\n");
}



SYS_PUBLIC void glDisablei(GLenum target,GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDisablei\x1b[0m\n");
}



SYS_PUBLIC void glDisableVertexAttribArray(GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDisableVertexAttribArray\x1b[0m\n");
}



SYS_PUBLIC void glDrawArrays(GLenum mode,GLint first,GLsizei count){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawArrays\x1b[0m\n");
}



SYS_PUBLIC void glDrawArraysInstanced(GLenum mode,GLint first,GLsizei count,GLsizei instancecount){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawArraysInstanced\x1b[0m\n");
}



SYS_PUBLIC void glDrawBuffer(GLenum buf){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawBuffer\x1b[0m\n");
}



SYS_PUBLIC void glDrawBuffers(GLsizei n,const GLenum* bufs){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawBuffers\x1b[0m\n");
}



SYS_PUBLIC void glDrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawElements\x1b[0m\n");
}



SYS_PUBLIC void glDrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawElementsBaseVertex\x1b[0m\n");
}



SYS_PUBLIC void glDrawElementsInstanced(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawElementsInstanced\x1b[0m\n");
}



SYS_PUBLIC void glDrawElementsInstancedBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount,GLint basevertex){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawElementsInstancedBaseVertex\x1b[0m\n");
}



SYS_PUBLIC void glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawRangeElements\x1b[0m\n");
}



SYS_PUBLIC void glDrawRangeElementsBaseVertex(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawRangeElementsBaseVertex\x1b[0m\n");
}



SYS_PUBLIC void glEnable(GLenum cap){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEnable\x1b[0m\n");
}



SYS_PUBLIC void glEnablei(GLenum target,GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEnablei\x1b[0m\n");
}



SYS_PUBLIC void glEnableVertexAttribArray(GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEnableVertexAttribArray\x1b[0m\n");
}



SYS_PUBLIC void glEndConditionalRender(void){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEndConditionalRender\x1b[0m\n");
}



SYS_PUBLIC void glEndQuery(GLenum target){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEndQuery\x1b[0m\n");
}



SYS_PUBLIC void glEndTransformFeedback(void){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEndTransformFeedback\x1b[0m\n");
}



SYS_PUBLIC GLsync glFenceSync(GLenum condition,GLbitfield flags){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFenceSync\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glFinish(void){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFinish\x1b[0m\n");
}



SYS_PUBLIC void glFlush(void){
	opengl_command_buffer_flush();
}



SYS_PUBLIC void glFlushMappedBufferRange(GLenum target,GLintptr offset,GLsizeiptr length){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFlushMappedBufferRange\x1b[0m\n");
}



SYS_PUBLIC void glFramebufferRenderbuffer(GLenum target,GLenum attachment,GLenum renderbuffertarget,GLuint renderbuffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFramebufferRenderbuffer\x1b[0m\n");
}



SYS_PUBLIC void glFramebufferTexture(GLenum target,GLenum attachment,GLuint texture,GLint level){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFramebufferTexture\x1b[0m\n");
}



SYS_PUBLIC void glFramebufferTexture1D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFramebufferTexture1D\x1b[0m\n");
}



SYS_PUBLIC void glFramebufferTexture2D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFramebufferTexture2D\x1b[0m\n");
}



SYS_PUBLIC void glFramebufferTexture3D(GLenum target,GLenum attachment,GLenum textarget,GLuint texture,GLint level,GLint zoffset){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFramebufferTexture3D\x1b[0m\n");
}



SYS_PUBLIC void glFramebufferTextureLayer(GLenum target,GLenum attachment,GLuint texture,GLint level,GLint layer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFramebufferTextureLayer\x1b[0m\n");
}



SYS_PUBLIC void glFrontFace(GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glFrontFace\x1b[0m\n");
}



SYS_PUBLIC void glGenBuffers(GLsizei n,GLuint* buffers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenBuffers\x1b[0m\n");
}



SYS_PUBLIC void glGenerateMipmap(GLenum target){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenerateMipmap\x1b[0m\n");
}



SYS_PUBLIC void glGenFramebuffers(GLsizei n,GLuint* framebuffers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenFramebuffers\x1b[0m\n");
}



SYS_PUBLIC void glGenQueries(GLsizei n,GLuint* ids){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenQueries\x1b[0m\n");
}



SYS_PUBLIC void glGenRenderbuffers(GLsizei n,GLuint* renderbuffers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenRenderbuffers\x1b[0m\n");
}



SYS_PUBLIC void glGenSamplers(GLsizei count,GLuint* samplers){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenSamplers\x1b[0m\n");
}



SYS_PUBLIC void glGenTextures(GLsizei n,GLuint* textures){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenTextures\x1b[0m\n");
}



SYS_PUBLIC void glGenVertexArrays(GLsizei n,GLuint* arrays){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGenVertexArrays\x1b[0m\n");
}



SYS_PUBLIC void glGetActiveAttrib(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetActiveAttrib\x1b[0m\n");
}



SYS_PUBLIC void glGetActiveUniform(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLint* size,GLenum* type,GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetActiveUniform\x1b[0m\n");
}



SYS_PUBLIC void glGetActiveUniformBlockiv(GLuint program,GLuint uniformBlockIndex,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetActiveUniformBlockiv\x1b[0m\n");
}



SYS_PUBLIC void glGetActiveUniformBlockName(GLuint program,GLuint uniformBlockIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformBlockName){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetActiveUniformBlockName\x1b[0m\n");
}



SYS_PUBLIC void glGetActiveUniformName(GLuint program,GLuint uniformIndex,GLsizei bufSize,GLsizei* length,GLchar* uniformName){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetActiveUniformName\x1b[0m\n");
}



SYS_PUBLIC void glGetActiveUniformsiv(GLuint program,GLsizei uniformCount,const GLuint* uniformIndices,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetActiveUniformsiv\x1b[0m\n");
}



SYS_PUBLIC void glGetAttachedShaders(GLuint program,GLsizei maxCount,GLsizei* count,GLuint* shaders){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetAttachedShaders\x1b[0m\n");
}



SYS_PUBLIC GLint glGetAttribLocation(GLuint program,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetAttribLocation\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glGetBooleani_v(GLenum target,GLuint index,GLboolean* data){
	_gl_get_parameter(target,index,data,OPENGL_PARAMETER_RETURN_TYPE_BOOL);
}



SYS_PUBLIC void glGetBooleanv(GLenum pname,GLboolean* data){
	_gl_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_BOOL);
}



SYS_PUBLIC void glGetBufferParameteri64v(GLenum target,GLenum pname,GLint64* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferParameteri64v\x1b[0m\n");
}



SYS_PUBLIC void glGetBufferParameteriv(GLenum target,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetBufferPointerv(GLenum target,GLenum pname,void* *params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferPointerv\x1b[0m\n");
}



SYS_PUBLIC void glGetBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferSubData\x1b[0m\n");
}



SYS_PUBLIC void glGetCompressedTexImage(GLenum target,GLint level,void* img){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetCompressedTexImage\x1b[0m\n");
}



SYS_PUBLIC void glGetDoublev(GLenum pname,GLdouble* data){
	_gl_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_DOUBLE);
}



SYS_PUBLIC GLenum glGetError(void){
	GLenum out=_gl_internal_state->gl_error;
	_gl_internal_state->gl_error=GL_NO_ERROR;
	return out;
}



SYS_PUBLIC void glGetFloatv(GLenum pname,GLfloat* data){
	_gl_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_FLOAT);
}



SYS_PUBLIC GLint glGetFragDataIndex(GLuint program,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetFragDataIndex\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLint glGetFragDataLocation(GLuint program,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetFragDataLocation\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glGetFramebufferAttachmentParameteriv(GLenum target,GLenum attachment,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetFramebufferAttachmentParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetInteger64i_v(GLenum target,GLuint index,GLint64* data){
	_gl_get_parameter(target,index,data,OPENGL_PARAMETER_RETURN_TYPE_INT64);
}



SYS_PUBLIC void glGetInteger64v(GLenum pname,GLint64* data){
	_gl_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_INT64);
}



SYS_PUBLIC void glGetIntegeri_v(GLenum target,GLuint index,GLint* data){
	_gl_get_parameter(target,index,data,OPENGL_PARAMETER_RETURN_TYPE_INT);
}



SYS_PUBLIC void glGetIntegerv(GLenum pname,GLint* data){
	_gl_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_INT);
}



SYS_PUBLIC void glGetMultisamplefv(GLenum pname,GLuint index,GLfloat* val){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetMultisamplefv\x1b[0m\n");
}



SYS_PUBLIC void glGetPointerv(GLenum pname,void* *params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetPointerv\x1b[0m\n");
}



SYS_PUBLIC void glGetProgramInfoLog(GLuint program,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramInfoLog\x1b[0m\n");
}



SYS_PUBLIC void glGetProgramiv(GLuint program,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv\x1b[0m\n");
}



SYS_PUBLIC void glGetQueryiv(GLenum target,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetQueryiv\x1b[0m\n");
}



SYS_PUBLIC void glGetQueryObjecti64v(GLuint id,GLenum pname,GLint64* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetQueryObjecti64v\x1b[0m\n");
}



SYS_PUBLIC void glGetQueryObjectiv(GLuint id,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetQueryObjectiv\x1b[0m\n");
}



SYS_PUBLIC void glGetQueryObjectui64v(GLuint id,GLenum pname,GLuint64* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetQueryObjectui64v\x1b[0m\n");
}



SYS_PUBLIC void glGetQueryObjectuiv(GLuint id,GLenum pname,GLuint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetQueryObjectuiv\x1b[0m\n");
}



SYS_PUBLIC void glGetRenderbufferParameteriv(GLenum target,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetRenderbufferParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetSamplerParameterfv(GLuint sampler,GLenum pname,GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetSamplerParameterfv\x1b[0m\n");
}



SYS_PUBLIC void glGetSamplerParameterIiv(GLuint sampler,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetSamplerParameterIiv\x1b[0m\n");
}



SYS_PUBLIC void glGetSamplerParameterIuiv(GLuint sampler,GLenum pname,GLuint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetSamplerParameterIuiv\x1b[0m\n");
}



SYS_PUBLIC void glGetSamplerParameteriv(GLuint sampler,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetSamplerParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetShaderInfoLog(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetShaderInfoLog\x1b[0m\n");
}



SYS_PUBLIC void glGetShaderiv(GLuint shader,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetShaderiv\x1b[0m\n");
}



SYS_PUBLIC void glGetShaderSource(GLuint shader,GLsizei bufSize,GLsizei* length,GLchar* source){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetShaderSource\x1b[0m\n");
}



SYS_PUBLIC const GLubyte* glGetString(GLenum name){
	switch (name){
		case GL_RENDERER:
			return (const GLubyte*)(_gl_internal_state->gl_renderer);
		case GL_SHADING_LANGUAGE_VERSION:
			return (const GLubyte*)(_gl_internal_state->gl_shading_language_version);
		case GL_VENDOR:
			return (const GLubyte*)(_gl_internal_state->gl_vendor);
		case GL_VERSION:
			return (const GLubyte*)(_gl_internal_state->gl_version);
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			return NULL;
	}
}



SYS_PUBLIC const GLubyte* glGetStringi(GLenum name,GLuint index){
	if (name!=GL_EXTENSIONS){
		_gl_internal_state->gl_error=GL_INVALID_ENUM;
		return NULL;
	}
	// no extensions supported
	return (const GLubyte*)"";
}



SYS_PUBLIC void glGetSynciv(GLsync sync,GLenum pname,GLsizei count,GLsizei* length,GLint* values){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetSynciv\x1b[0m\n");
}



SYS_PUBLIC void glGetTexImage(GLenum target,GLint level,GLenum format,GLenum type,void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexImage\x1b[0m\n");
}



SYS_PUBLIC void glGetTexLevelParameterfv(GLenum target,GLint level,GLenum pname,GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexLevelParameterfv\x1b[0m\n");
}



SYS_PUBLIC void glGetTexLevelParameteriv(GLenum target,GLint level,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexLevelParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetTexParameterfv(GLenum target,GLenum pname,GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexParameterfv\x1b[0m\n");
}



SYS_PUBLIC void glGetTexParameterIiv(GLenum target,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexParameterIiv\x1b[0m\n");
}



SYS_PUBLIC void glGetTexParameterIuiv(GLenum target,GLenum pname,GLuint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexParameterIuiv\x1b[0m\n");
}



SYS_PUBLIC void glGetTexParameteriv(GLenum target,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTexParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetTransformFeedbackVarying(GLuint program,GLuint index,GLsizei bufSize,GLsizei* length,GLsizei* size,GLenum* type,GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetTransformFeedbackVarying\x1b[0m\n");
}



SYS_PUBLIC GLuint glGetUniformBlockIndex(GLuint program,const GLchar* uniformBlockName){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetUniformBlockIndex\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glGetUniformfv(GLuint program,GLint location,GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetUniformfv\x1b[0m\n");
}



SYS_PUBLIC void glGetUniformIndices(GLuint program,GLsizei uniformCount,const GLchar*const* uniformNames,GLuint* uniformIndices){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetUniformIndices\x1b[0m\n");
}



SYS_PUBLIC void glGetUniformiv(GLuint program,GLint location,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetUniformiv\x1b[0m\n");
}



SYS_PUBLIC GLint glGetUniformLocation(GLuint program,const GLchar* name){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetUniformLocation\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glGetUniformuiv(GLuint program,GLint location,GLuint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetUniformuiv\x1b[0m\n");
}



SYS_PUBLIC void glGetVertexAttribdv(GLuint index,GLenum pname,GLdouble* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribdv\x1b[0m\n");
}



SYS_PUBLIC void glGetVertexAttribfv(GLuint index,GLenum pname,GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribfv\x1b[0m\n");
}



SYS_PUBLIC void glGetVertexAttribIiv(GLuint index,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribIiv\x1b[0m\n");
}



SYS_PUBLIC void glGetVertexAttribIuiv(GLuint index,GLenum pname,GLuint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribIuiv\x1b[0m\n");
}



SYS_PUBLIC void glGetVertexAttribiv(GLuint index,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribiv\x1b[0m\n");
}



SYS_PUBLIC void glGetVertexAttribPointerv(GLuint index,GLenum pname,void* *pointer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribPointerv\x1b[0m\n");
}



SYS_PUBLIC void glHint(GLenum target,GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glHint\x1b[0m\n");
}



SYS_PUBLIC GLboolean glIsBuffer(GLuint buffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsBuffer\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsEnabled(GLenum cap){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsEnabled\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsEnabledi(GLenum target,GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsEnabledi\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsFramebuffer(GLuint framebuffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsFramebuffer\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsProgram(GLuint program){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsProgram\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsQuery(GLuint id){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsQuery\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsRenderbuffer(GLuint renderbuffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsRenderbuffer\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsSampler(GLuint sampler){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsSampler\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsShader(GLuint shader){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsShader\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsSync(GLsync sync){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsSync\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsTexture(GLuint texture){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsTexture\x1b[0m\n");
	return 0;
}



SYS_PUBLIC GLboolean glIsVertexArray(GLuint array){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glIsVertexArray\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glLineWidth(GLfloat width){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glLineWidth\x1b[0m\n");
}



SYS_PUBLIC void glLinkProgram(GLuint program){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glLinkProgram\x1b[0m\n");
}



SYS_PUBLIC void glLogicOp(GLenum opcode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glLogicOp\x1b[0m\n");
}



SYS_PUBLIC void* glMapBuffer(GLenum target,GLenum access){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glMapBuffer\x1b[0m\n");
	return NULL;
}



SYS_PUBLIC void* glMapBufferRange(GLenum target,GLintptr offset,GLsizeiptr length,GLbitfield access){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glMapBufferRange\x1b[0m\n");
	return NULL;
}



SYS_PUBLIC void glMultiDrawArrays(GLenum mode,const GLint* first,const GLsizei* count,GLsizei drawcount){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glMultiDrawArrays\x1b[0m\n");
}



SYS_PUBLIC void glMultiDrawElements(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glMultiDrawElements\x1b[0m\n");
}



SYS_PUBLIC void glMultiDrawElementsBaseVertex(GLenum mode,const GLsizei* count,GLenum type,const void* const*indices,GLsizei drawcount,const GLint* basevertex){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glMultiDrawElementsBaseVertex\x1b[0m\n");
}



SYS_PUBLIC void glPixelStoref(GLenum pname,GLfloat param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPixelStoref\x1b[0m\n");
}



SYS_PUBLIC void glPixelStorei(GLenum pname,GLint param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPixelStorei\x1b[0m\n");
}



SYS_PUBLIC void glPointParameterf(GLenum pname,GLfloat param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPointParameterf\x1b[0m\n");
}



SYS_PUBLIC void glPointParameterfv(GLenum pname,const GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPointParameterfv\x1b[0m\n");
}



SYS_PUBLIC void glPointParameteri(GLenum pname,GLint param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPointParameteri\x1b[0m\n");
}



SYS_PUBLIC void glPointParameteriv(GLenum pname,const GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPointParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glPointSize(GLfloat size){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPointSize\x1b[0m\n");
}



SYS_PUBLIC void glPolygonMode(GLenum face,GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPolygonMode\x1b[0m\n");
}



SYS_PUBLIC void glPolygonOffset(GLfloat factor,GLfloat units){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPolygonOffset\x1b[0m\n");
}



SYS_PUBLIC void glPrimitiveRestartIndex(GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glPrimitiveRestartIndex\x1b[0m\n");
}



SYS_PUBLIC void glProvokingVertex(GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glProvokingVertex\x1b[0m\n");
}



SYS_PUBLIC void glQueryCounter(GLuint id,GLenum target){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glQueryCounter\x1b[0m\n");
}



SYS_PUBLIC void glReadBuffer(GLenum src){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glReadBuffer\x1b[0m\n");
}



SYS_PUBLIC void glReadPixels(GLint x,GLint y,GLsizei width,GLsizei height,GLenum format,GLenum type,void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glReadPixels\x1b[0m\n");
}



SYS_PUBLIC void glRenderbufferStorage(GLenum target,GLenum internalformat,GLsizei width,GLsizei height){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glRenderbufferStorage\x1b[0m\n");
}



SYS_PUBLIC void glRenderbufferStorageMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glRenderbufferStorageMultisample\x1b[0m\n");
}



SYS_PUBLIC void glSampleCoverage(GLfloat value,GLboolean invert){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSampleCoverage\x1b[0m\n");
}



SYS_PUBLIC void glSampleMaski(GLuint maskNumber,GLbitfield mask){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSampleMaski\x1b[0m\n");
}



SYS_PUBLIC void glSamplerParameterf(GLuint sampler,GLenum pname,GLfloat param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSamplerParameterf\x1b[0m\n");
}



SYS_PUBLIC void glSamplerParameterfv(GLuint sampler,GLenum pname,const GLfloat* param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSamplerParameterfv\x1b[0m\n");
}



SYS_PUBLIC void glSamplerParameteri(GLuint sampler,GLenum pname,GLint param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSamplerParameteri\x1b[0m\n");
}



SYS_PUBLIC void glSamplerParameterIiv(GLuint sampler,GLenum pname,const GLint* param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSamplerParameterIiv\x1b[0m\n");
}



SYS_PUBLIC void glSamplerParameterIuiv(GLuint sampler,GLenum pname,const GLuint* param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSamplerParameterIuiv\x1b[0m\n");
}



SYS_PUBLIC void glSamplerParameteriv(GLuint sampler,GLenum pname,const GLint* param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glSamplerParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glScissor(GLint x,GLint y,GLsizei width,GLsizei height){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glScissor\x1b[0m\n");
}



SYS_PUBLIC void glShaderSource(GLuint shader,GLsizei count,const GLchar*const* string,const GLint* length){
	opengl_shader_state_t* state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER);
	if (!state){
		return;
	}
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	_shader_state_remove_sources(state);
	if (!count){
		return;
	}
	state->source_count=count;
	state->sources=sys_heap_alloc(NULL,count*sizeof(opengl_shader_source_t));
	for (GLsizei i=0;i<count;i++){
		opengl_shader_source_t* src=state->sources+i;
		src->length=(!length||length[i]<0?sys_string_length(string[i]):length[i]);
		src->data=sys_heap_alloc(NULL,src->length+1);
		sys_memory_copy(string[i],src->data,src->length);
		src->data[src->length]=0;
	}
}



SYS_PUBLIC void glStencilFunc(GLenum func,GLint ref,GLuint mask){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glStencilFunc\x1b[0m\n");
}



SYS_PUBLIC void glStencilFuncSeparate(GLenum face,GLenum func,GLint ref,GLuint mask){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glStencilFuncSeparate\x1b[0m\n");
}



SYS_PUBLIC void glStencilMask(GLuint mask){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glStencilMask\x1b[0m\n");
}



SYS_PUBLIC void glStencilMaskSeparate(GLenum face,GLuint mask){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glStencilMaskSeparate\x1b[0m\n");
}



SYS_PUBLIC void glStencilOp(GLenum fail,GLenum zfail,GLenum zpass){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glStencilOp\x1b[0m\n");
}



SYS_PUBLIC void glStencilOpSeparate(GLenum face,GLenum sfail,GLenum dpfail,GLenum dppass){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glStencilOpSeparate\x1b[0m\n");
}



SYS_PUBLIC void glTexBuffer(GLenum target,GLenum internalformat,GLuint buffer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexBuffer\x1b[0m\n");
}



SYS_PUBLIC void glTexImage1D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLint border,GLenum format,GLenum type,const void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexImage1D\x1b[0m\n");
}



SYS_PUBLIC void glTexImage2D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexImage2D\x1b[0m\n");
}



SYS_PUBLIC void glTexImage2DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLboolean fixedsamplelocations){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexImage2DMultisample\x1b[0m\n");
}



SYS_PUBLIC void glTexImage3D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLsizei depth,GLint border,GLenum format,GLenum type,const void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexImage3D\x1b[0m\n");
}



SYS_PUBLIC void glTexImage3DMultisample(GLenum target,GLsizei samples,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth,GLboolean fixedsamplelocations){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexImage3DMultisample\x1b[0m\n");
}



SYS_PUBLIC void glTexParameterf(GLenum target,GLenum pname,GLfloat param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexParameterf\x1b[0m\n");
}



SYS_PUBLIC void glTexParameterfv(GLenum target,GLenum pname,const GLfloat* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexParameterfv\x1b[0m\n");
}



SYS_PUBLIC void glTexParameteri(GLenum target,GLenum pname,GLint param){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexParameteri\x1b[0m\n");
}



SYS_PUBLIC void glTexParameterIiv(GLenum target,GLenum pname,const GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexParameterIiv\x1b[0m\n");
}



SYS_PUBLIC void glTexParameterIuiv(GLenum target,GLenum pname,const GLuint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexParameterIuiv\x1b[0m\n");
}



SYS_PUBLIC void glTexParameteriv(GLenum target,GLenum pname,const GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexSubImage1D\x1b[0m\n");
}



SYS_PUBLIC void glTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexSubImage2D\x1b[0m\n");
}



SYS_PUBLIC void glTexSubImage3D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLint zoffset,GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type,const void* pixels){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTexSubImage3D\x1b[0m\n");
}



SYS_PUBLIC void glTransformFeedbackVaryings(GLuint program,GLsizei count,const GLchar*const* varyings,GLenum bufferMode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glTransformFeedbackVaryings\x1b[0m\n");
}



SYS_PUBLIC void glUniform1f(GLint location,GLfloat v0){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform1f\x1b[0m\n");
}



SYS_PUBLIC void glUniform1fv(GLint location,GLsizei count,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform1fv\x1b[0m\n");
}



SYS_PUBLIC void glUniform1i(GLint location,GLint v0){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform1i\x1b[0m\n");
}



SYS_PUBLIC void glUniform1iv(GLint location,GLsizei count,const GLint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform1iv\x1b[0m\n");
}



SYS_PUBLIC void glUniform1ui(GLint location,GLuint v0){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform1ui\x1b[0m\n");
}



SYS_PUBLIC void glUniform1uiv(GLint location,GLsizei count,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform1uiv\x1b[0m\n");
}



SYS_PUBLIC void glUniform2f(GLint location,GLfloat v0,GLfloat v1){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform2f\x1b[0m\n");
}



SYS_PUBLIC void glUniform2fv(GLint location,GLsizei count,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform2fv\x1b[0m\n");
}



SYS_PUBLIC void glUniform2i(GLint location,GLint v0,GLint v1){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform2i\x1b[0m\n");
}



SYS_PUBLIC void glUniform2iv(GLint location,GLsizei count,const GLint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform2iv\x1b[0m\n");
}



SYS_PUBLIC void glUniform2ui(GLint location,GLuint v0,GLuint v1){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform2ui\x1b[0m\n");
}



SYS_PUBLIC void glUniform2uiv(GLint location,GLsizei count,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform2uiv\x1b[0m\n");
}



SYS_PUBLIC void glUniform3f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform3f\x1b[0m\n");
}



SYS_PUBLIC void glUniform3fv(GLint location,GLsizei count,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform3fv\x1b[0m\n");
}



SYS_PUBLIC void glUniform3i(GLint location,GLint v0,GLint v1,GLint v2){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform3i\x1b[0m\n");
}



SYS_PUBLIC void glUniform3iv(GLint location,GLsizei count,const GLint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform3iv\x1b[0m\n");
}



SYS_PUBLIC void glUniform3ui(GLint location,GLuint v0,GLuint v1,GLuint v2){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform3ui\x1b[0m\n");
}



SYS_PUBLIC void glUniform3uiv(GLint location,GLsizei count,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform3uiv\x1b[0m\n");
}



SYS_PUBLIC void glUniform4f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform4f\x1b[0m\n");
}



SYS_PUBLIC void glUniform4fv(GLint location,GLsizei count,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform4fv\x1b[0m\n");
}



SYS_PUBLIC void glUniform4i(GLint location,GLint v0,GLint v1,GLint v2,GLint v3){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform4i\x1b[0m\n");
}



SYS_PUBLIC void glUniform4iv(GLint location,GLsizei count,const GLint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform4iv\x1b[0m\n");
}



SYS_PUBLIC void glUniform4ui(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform4ui\x1b[0m\n");
}



SYS_PUBLIC void glUniform4uiv(GLint location,GLsizei count,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniform4uiv\x1b[0m\n");
}



SYS_PUBLIC void glUniformBlockBinding(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformBlockBinding\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix2fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix2x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix2x3fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix2x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix2x4fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix3fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix3x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix3x2fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix3x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix3x4fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix4fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix4x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix4x2fv\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix4x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformMatrix4x3fv\x1b[0m\n");
}



SYS_PUBLIC GLboolean glUnmapBuffer(GLenum target){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUnmapBuffer\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glUseProgram(GLuint program){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUseProgram\x1b[0m\n");
}



SYS_PUBLIC void glValidateProgram(GLuint program){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glValidateProgram\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1d(GLuint index,GLdouble x){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib1d\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1dv(GLuint index,const GLdouble* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib1dv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1f(GLuint index,GLfloat x){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib1f\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1fv(GLuint index,const GLfloat* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib1fv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1s(GLuint index,GLshort x){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib1s\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1sv(GLuint index,const GLshort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib1sv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib2d(GLuint index,GLdouble x,GLdouble y){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib2d\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib2dv(GLuint index,const GLdouble* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib2dv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib2f(GLuint index,GLfloat x,GLfloat y){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib2f\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib2fv(GLuint index,const GLfloat* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib2fv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib2s(GLuint index,GLshort x,GLshort y){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib2s\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib2sv(GLuint index,const GLshort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib2sv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib3d(GLuint index,GLdouble x,GLdouble y,GLdouble z){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib3d\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib3dv(GLuint index,const GLdouble* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib3dv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib3f(GLuint index,GLfloat x,GLfloat y,GLfloat z){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib3f\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib3fv(GLuint index,const GLfloat* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib3fv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib3s(GLuint index,GLshort x,GLshort y,GLshort z){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib3s\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib3sv(GLuint index,const GLshort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib3sv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4bv(GLuint index,const GLbyte* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4bv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4d(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4d\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4dv(GLuint index,const GLdouble* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4dv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4f(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4f\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4fv(GLuint index,const GLfloat* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4fv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4iv(GLuint index,const GLint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4iv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Nbv(GLuint index,const GLbyte* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Nbv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Niv(GLuint index,const GLint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Niv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Nsv(GLuint index,const GLshort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Nsv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Nub(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Nub\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Nubv(GLuint index,const GLubyte* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Nubv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Nuiv(GLuint index,const GLuint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Nuiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4Nusv(GLuint index,const GLushort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4Nusv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4s(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4s\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4sv(GLuint index,const GLshort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4sv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4ubv(GLuint index,const GLubyte* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4ubv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4uiv(GLuint index,const GLuint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib4usv(GLuint index,const GLushort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttrib4usv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribDivisor(GLuint index,GLuint divisor){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribDivisor\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI1i(GLuint index,GLint x){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI1i\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI1iv(GLuint index,const GLint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI1iv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI1ui(GLuint index,GLuint x){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI1ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI1uiv(GLuint index,const GLuint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI1uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI2i(GLuint index,GLint x,GLint y){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI2i\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI2iv(GLuint index,const GLint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI2iv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI2ui(GLuint index,GLuint x,GLuint y){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI2ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI2uiv(GLuint index,const GLuint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI2uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI3i(GLuint index,GLint x,GLint y,GLint z){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI3i\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI3iv(GLuint index,const GLint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI3iv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI3ui(GLuint index,GLuint x,GLuint y,GLuint z){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI3ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI3uiv(GLuint index,const GLuint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI3uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4bv(GLuint index,const GLbyte* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4bv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4i(GLuint index,GLint x,GLint y,GLint z,GLint w){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4i\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4iv(GLuint index,const GLint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4iv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4sv(GLuint index,const GLshort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4sv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4ubv(GLuint index,const GLubyte* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4ubv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4ui(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4uiv(GLuint index,const GLuint* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribI4usv(GLuint index,const GLushort* v){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribI4usv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribIPointer(GLuint index,GLint size,GLenum type,GLsizei stride,const void* pointer){
	if (size<1||size>4){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (size==GL_BGRA&&type!=GL_UNSIGNED_BYTE&&type!=GL_INT_2_10_10_10_REV&&type!=GL_UNSIGNED_INT_2_10_10_10_REV){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if ((type==GL_INT_2_10_10_10_REV||type==GL_UNSIGNED_INT_2_10_10_10_REV)&&size!=4&&size==GL_BGRA){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (pointer&&/*no array buffer is bound*/1){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (index>=_gl_max_vertex_attribs){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (stride<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribIPointer\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP1ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP1ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP1uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP1uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP2ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP2ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP2uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP2uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP3ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP3ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP3uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP3uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP4ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP4ui\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribP4uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribP4uiv\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttribPointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer){
	if (size<1||size>4){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (size==GL_BGRA&&type!=GL_UNSIGNED_BYTE&&type!=GL_INT_2_10_10_10_REV&&type!=GL_UNSIGNED_INT_2_10_10_10_REV){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if ((type==GL_INT_2_10_10_10_REV||type==GL_UNSIGNED_INT_2_10_10_10_REV)&&size!=4&&size==GL_BGRA){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (size==GL_BGRA&&!normalized){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (pointer&&/*no array buffer is bound*/1){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (index>=_gl_max_vertex_attribs){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (stride<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glVertexAttribPointer\x1b[0m\n");
}



SYS_PUBLIC void glViewport(GLint x,GLint y,GLsizei width,GLsizei height){
	_gl_internal_state->gl_viewport[0]=x;
	_gl_internal_state->gl_viewport[1]=y;
	_gl_internal_state->gl_viewport[2]=width;
	_gl_internal_state->gl_viewport[3]=height;
	opengl_protocol_set_viewport_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_SET_VIEWPORT,
		.header.length=sizeof(opengl_protocol_set_viewport_t),
		.tx=x+width/2.0f,
		.ty=y+height/2.0f,
		.sx=width/2.0f,
		.sy=-height/2.0f
	};
	opengl_command_buffer_push_single(&(command.header));
}



SYS_PUBLIC void glWaitSync(GLsync sync,GLbitfield flags,GLuint64 timeout){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glWaitSync\x1b[0m\n");
}
