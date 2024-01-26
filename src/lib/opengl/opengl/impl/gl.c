#include <GL/gl.h>
#include <glsl/ast.h>
#include <glsl/compiler.h>
#include <glsl/error.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>
#include <glsl/parser.h>
#include <glsl/preprocessor.h>
#include <opengl/_internal/state.h>
#include <opengl/command_buffer.h>
#include <opengl/config.h>
#include <opengl/protocol.h>
#include <src_lib_opengl_rsrc_glsl_frag_setup_glsl.h>
#include <src_lib_opengl_rsrc_glsl_global_setup_glsl.h>
#include <src_lib_opengl_rsrc_glsl_vert_setup_glsl.h>
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



static void* _get_handle(GLuint handle,opengl_handle_type_t type,_Bool always_invalid_operation){
	if (handle>=_gl_internal_state->handle_count||!_gl_internal_state->handles[handle]){
		_gl_internal_state->gl_error=(always_invalid_operation?GL_INVALID_OPERATION:GL_INVALID_VALUE);
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



static void _get_parameter(GLenum param,u64 index,void* out,u32 out_type){
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
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_ALIASED_LINE_WIDTH_RANGE\x1b[0m\n");
			return;
		case GL_SMOOTH_LINE_WIDTH_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SMOOTH_LINE_WIDTH_RANGE\x1b[0m\n");
			return;
		case GL_SMOOTH_LINE_WIDTH_GRANULARITY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SMOOTH_LINE_WIDTH_GRANULARITY\x1b[0m\n");
			return;
		case GL_ARRAY_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_ARRAY_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_BLEND:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND\x1b[0m\n");
			return;
		case GL_BLEND_COLOR:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_COLOR\x1b[0m\n");
			return;
		case GL_BLEND_DST_ALPHA:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_DST_ALPHA\x1b[0m\n");
			return;
		case GL_BLEND_DST_RGB:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_DST_RGB\x1b[0m\n");
			return;
		case GL_BLEND_EQUATION_RGB:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_EQUATION_RGB\x1b[0m\n");
			return;
		case GL_BLEND_EQUATION_ALPHA:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_EQUATION_ALPHA\x1b[0m\n");
			return;
		case GL_BLEND_SRC_ALPHA:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_SRC_ALPHA\x1b[0m\n");
			return;
		case GL_BLEND_SRC_RGB:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_BLEND_SRC_RGB\x1b[0m\n");
			return;
		case GL_COLOR_CLEAR_VALUE:
			type=OPENGL_PARAMETER_TYPE_COLOR_OR_NORMAL;
			length=4;
			values=_gl_internal_state->gl_clear_color_value;
			break;
		case GL_COLOR_LOGIC_OP:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_COLOR_LOGIC_OP\x1b[0m\n");
			return;
		case GL_COLOR_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_COLOR_WRITEMASK\x1b[0m\n");
			return;
		case GL_COMPRESSED_TEXTURE_FORMATS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_COMPRESSED_TEXTURE_FORMATS\x1b[0m\n");
			return;
		case GL_CULL_FACE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_CULL_FACE\x1b[0m\n");
			return;
		case GL_CURRENT_PROGRAM:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_CURRENT_PROGRAM\x1b[0m\n");
			return;
		case GL_DEPTH_CLEAR_VALUE:
			type=OPENGL_PARAMETER_TYPE_DOUBLE;
			length=1;
			values=&_gl_internal_state->gl_clear_depth_value;
			break;
		case GL_DEPTH_FUNC:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DEPTH_FUNC\x1b[0m\n");
			return;
		case GL_DEPTH_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DEPTH_RANGE\x1b[0m\n");
			return;
		case GL_DEPTH_TEST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DEPTH_TEST\x1b[0m\n");
			return;
		case GL_DEPTH_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DEPTH_WRITEMASK\x1b[0m\n");
			return;
		case GL_DITHER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DITHER\x1b[0m\n");
			return;
		case GL_DOUBLEBUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DOUBLEBUFFER\x1b[0m\n");
			return;
		case GL_DRAW_BUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DRAW_BUFFER\x1b[0m\n");
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
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DRAW_BUFFER%u\x1b[0m\n",param-GL_DRAW_BUFFER0);
			return;
		case GL_DRAW_FRAMEBUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_DRAW_FRAMEBUFFER_BINDING\x1b[0m\n");
			return;
		case GL_READ_FRAMEBUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_READ_FRAMEBUFFER_BINDING\x1b[0m\n");
			return;
		case GL_ELEMENT_ARRAY_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_ELEMENT_ARRAY_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_RENDERBUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_RENDERBUFFER_BINDING\x1b[0m\n");
			return;
		case GL_FRAGMENT_SHADER_DERIVATIVE_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_FRAGMENT_SHADER_DERIVATIVE_HINT\x1b[0m\n");
			return;
		case GL_LINE_SMOOTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_LINE_SMOOTH\x1b[0m\n");
			return;
		case GL_LINE_SMOOTH_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_LINE_SMOOTH_HINT\x1b[0m\n");
			return;
		case GL_LINE_WIDTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_LINE_WIDTH\x1b[0m\n");
			return;
		case GL_LOGIC_OP_MODE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_LOGIC_OP_MODE\x1b[0m\n");
			return;
		case GL_MAX_3D_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_3D_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_CLIP_DISTANCES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_CLIP_DISTANCES\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
			local_value=OPENGL_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_COMBINED_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_COMBINED_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_CUBE_MAP_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_DRAW_BUFFERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_DRAW_BUFFERS\x1b[0m\n");
			return;
		case GL_MAX_DUAL_SOURCE_DRAW_BUFFERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_DUAL_SOURCE_DRAW_BUFFERS\x1b[0m\n");
			return;
		case GL_MAX_ELEMENTS_INDICES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_ELEMENTS_INDICES\x1b[0m\n");
			return;
		case GL_MAX_ELEMENTS_VERTICES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_ELEMENTS_VERTICES\x1b[0m\n");
			return;
		case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_FRAGMENT_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_FRAGMENT_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_FRAGMENT_INPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MIN_PROGRAM_TEXEL_OFFSET:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MIN_PROGRAM_TEXEL_OFFSET\x1b[0m\n");
			return;
		case GL_MAX_PROGRAM_TEXEL_OFFSET:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_PROGRAM_TEXEL_OFFSET\x1b[0m\n");
			return;
		case GL_MAX_RECTANGLE_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_RECTANGLE_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_IMAGE_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_TEXTURE_IMAGE_UNITS\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_LOD_BIAS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_TEXTURE_LOD_BIAS\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_TEXTURE_SIZE\x1b[0m\n");
			return;
		case GL_MAX_RENDERBUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_RENDERBUFFER_SIZE\x1b[0m\n");
			return;
		case GL_MAX_ARRAY_TEXTURE_LAYERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_ARRAY_TEXTURE_LAYERS\x1b[0m\n");
			return;
		case GL_MAX_TEXTURE_BUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_TEXTURE_BUFFER_SIZE\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_ATTRIBS:
			local_value=OPENGL_MAX_VERTEX_ATTRIBUTES;
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&local_value;
			break;
		case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_VERTEX_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_VERTEX_OUTPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_UNIFORM_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_GEOMETRY_UNIFORM_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_SAMPLE_MASK_WORDS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_SAMPLE_MASK_WORDS\x1b[0m\n");
			return;
		case GL_MAX_COLOR_TEXTURE_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_COLOR_TEXTURE_SAMPLES\x1b[0m\n");
			return;
		case GL_MAX_DEPTH_TEXTURE_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_DEPTH_TEXTURE_SAMPLES\x1b[0m\n");
			return;
		case GL_MAX_INTEGER_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_INTEGER_SAMPLES\x1b[0m\n");
			return;
		case GL_MAX_SERVER_WAIT_TIMEOUT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_SERVER_WAIT_TIMEOUT\x1b[0m\n");
			return;
		case GL_MAX_UNIFORM_BUFFER_BINDINGS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_UNIFORM_BUFFER_BINDINGS\x1b[0m\n");
			return;
		case GL_MAX_UNIFORM_BLOCK_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_UNIFORM_BLOCK_SIZE\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT\x1b[0m\n");
			return;
		case GL_MAX_VERTEX_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_VERTEX_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_UNIFORM_BLOCKS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_GEOMETRY_UNIFORM_BLOCKS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_INPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_GEOMETRY_INPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_GEOMETRY_OUTPUT_COMPONENTS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_GEOMETRY_OUTPUT_COMPONENTS\x1b[0m\n");
			return;
		case GL_MAX_VIEWPORT_DIMS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_MAX_VIEWPORT_DIMS\x1b[0m\n");
			return;
		case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_NUM_COMPRESSED_TEXTURE_FORMATS\x1b[0m\n");
			return;
		case GL_PACK_ALIGNMENT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_ALIGNMENT\x1b[0m\n");
			return;
		case GL_PACK_IMAGE_HEIGHT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_IMAGE_HEIGHT\x1b[0m\n");
			return;
		case GL_PACK_LSB_FIRST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_LSB_FIRST\x1b[0m\n");
			return;
		case GL_PACK_ROW_LENGTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_ROW_LENGTH\x1b[0m\n");
			return;
		case GL_PACK_SKIP_IMAGES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_SKIP_IMAGES\x1b[0m\n");
			return;
		case GL_PACK_SKIP_PIXELS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_SKIP_PIXELS\x1b[0m\n");
			return;
		case GL_PACK_SKIP_ROWS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_SKIP_ROWS\x1b[0m\n");
			return;
		case GL_PACK_SWAP_BYTES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PACK_SWAP_BYTES\x1b[0m\n");
			return;
		case GL_PIXEL_PACK_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PIXEL_PACK_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_PIXEL_UNPACK_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PIXEL_UNPACK_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_POINT_FADE_THRESHOLD_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POINT_FADE_THRESHOLD_SIZE\x1b[0m\n");
			return;
		case GL_PRIMITIVE_RESTART_INDEX:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PRIMITIVE_RESTART_INDEX\x1b[0m\n");
			return;
		case GL_PROGRAM_POINT_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PROGRAM_POINT_SIZE\x1b[0m\n");
			return;
		case GL_PROVOKING_VERTEX:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_PROVOKING_VERTEX\x1b[0m\n");
			return;
		case GL_POINT_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POINT_SIZE\x1b[0m\n");
			return;
		case GL_POINT_SIZE_GRANULARITY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POINT_SIZE_GRANULARITY\x1b[0m\n");
			return;
		case GL_POINT_SIZE_RANGE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POINT_SIZE_RANGE\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_FACTOR:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_OFFSET_FACTOR\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_UNITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_OFFSET_UNITS\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_FILL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_OFFSET_FILL\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_LINE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_OFFSET_LINE\x1b[0m\n");
			return;
		case GL_POLYGON_OFFSET_POINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_OFFSET_POINT\x1b[0m\n");
			return;
		case GL_POLYGON_SMOOTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_SMOOTH\x1b[0m\n");
			return;
		case GL_POLYGON_SMOOTH_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_POLYGON_SMOOTH_HINT\x1b[0m\n");
			return;
		case GL_READ_BUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_READ_BUFFER\x1b[0m\n");
			return;
		case GL_SAMPLE_BUFFERS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SAMPLE_BUFFERS\x1b[0m\n");
			return;
		case GL_SAMPLE_COVERAGE_VALUE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SAMPLE_COVERAGE_VALUE\x1b[0m\n");
			return;
		case GL_SAMPLE_COVERAGE_INVERT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SAMPLE_COVERAGE_INVERT\x1b[0m\n");
			return;
		case GL_SAMPLER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SAMPLER_BINDING\x1b[0m\n");
			return;
		case GL_SAMPLES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SAMPLES\x1b[0m\n");
			return;
		case GL_SCISSOR_BOX:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SCISSOR_BOX\x1b[0m\n");
			return;
		case GL_SCISSOR_TEST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SCISSOR_TEST\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_FUNC:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_FUNC\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_PASS_DEPTH_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_PASS_DEPTH_PASS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_PASS_DEPTH_PASS\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_REF:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_REF\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_VALUE_MASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_VALUE_MASK\x1b[0m\n");
			return;
		case GL_STENCIL_BACK_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_BACK_WRITEMASK\x1b[0m\n");
			return;
		case GL_STENCIL_CLEAR_VALUE:
			type=OPENGL_PARAMETER_TYPE_INT;
			length=1;
			values=&_gl_internal_state->gl_clear_stencil_value;
			break;
		case GL_STENCIL_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_FUNC:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_FUNC\x1b[0m\n");
			return;
		case GL_STENCIL_PASS_DEPTH_FAIL:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_PASS_DEPTH_FAIL\x1b[0m\n");
			return;
		case GL_STENCIL_PASS_DEPTH_PASS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_PASS_DEPTH_PASS\x1b[0m\n");
			return;
		case GL_STENCIL_REF:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_REF\x1b[0m\n");
			return;
		case GL_STENCIL_TEST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_TEST\x1b[0m\n");
			return;
		case GL_STENCIL_VALUE_MASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_VALUE_MASK\x1b[0m\n");
			return;
		case GL_STENCIL_WRITEMASK:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STENCIL_WRITEMASK\x1b[0m\n");
			return;
		case GL_STEREO:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_STEREO\x1b[0m\n");
			return;
		case GL_SUBPIXEL_BITS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_SUBPIXEL_BITS\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_1D:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_1D\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_1D_ARRAY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_1D_ARRAY\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_2D\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D_ARRAY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_2D_ARRAY\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D_MULTISAMPLE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_2D_MULTISAMPLE\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_3D:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_3D\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_BUFFER:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_BUFFER\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_CUBE_MAP:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_CUBE_MAP\x1b[0m\n");
			return;
		case GL_TEXTURE_BINDING_RECTANGLE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_BINDING_RECTANGLE\x1b[0m\n");
			return;
		case GL_TEXTURE_COMPRESSION_HINT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TEXTURE_COMPRESSION_HINT\x1b[0m\n");
			return;
		case GL_TIMESTAMP:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TIMESTAMP\x1b[0m\n");
			return;
		case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TRANSFORM_FEEDBACK_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_TRANSFORM_FEEDBACK_BUFFER_START:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TRANSFORM_FEEDBACK_BUFFER_START\x1b[0m\n");
			return;
		case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_TRANSFORM_FEEDBACK_BUFFER_SIZE\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_BINDING:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNIFORM_BUFFER_BINDING\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_START:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNIFORM_BUFFER_START\x1b[0m\n");
			return;
		case GL_UNIFORM_BUFFER_SIZE:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNIFORM_BUFFER_SIZE\x1b[0m\n");
			return;
		case GL_UNPACK_ALIGNMENT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_ALIGNMENT\x1b[0m\n");
			return;
		case GL_UNPACK_IMAGE_HEIGHT:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_IMAGE_HEIGHT\x1b[0m\n");
			return;
		case GL_UNPACK_LSB_FIRST:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_LSB_FIRST\x1b[0m\n");
			return;
		case GL_UNPACK_ROW_LENGTH:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_ROW_LENGTH\x1b[0m\n");
			return;
		case GL_UNPACK_SKIP_IMAGES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_SKIP_IMAGES\x1b[0m\n");
			return;
		case GL_UNPACK_SKIP_PIXELS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_SKIP_PIXELS\x1b[0m\n");
			return;
		case GL_UNPACK_SKIP_ROWS:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_SKIP_ROWS\x1b[0m\n");
			return;
		case GL_UNPACK_SWAP_BYTES:
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_UNPACK_SWAP_BYTES\x1b[0m\n");
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
			sys_io_print("\x1b[38;2;231;72;86mUnimplemented: _get_parameter.GL_CONTEXT_FLAGS\x1b[0m\n");
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



static void _sync_state(void){
	GLenum error=_gl_internal_state->gl_error;
	if (!_gl_internal_state->gl_used_vertex_array){
		goto _skip_vertex_array_sync;
	}
	opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		goto _skip_vertex_array_sync;
	}
	if (!state->needs_update){
		if (_gl_internal_state->gl_bound_vertex_array==state->header.index||!state->driver_handle){
			goto _skip_vertex_array_sync;
		}
		opengl_protocol_update_vertex_array_t command={
			.header.type=OPENGL_PROTOCOL_TYPE_UPDATE_VERTEX_ARRAY,
			.header.length=sizeof(opengl_protocol_update_vertex_array_t)-32*sizeof(opengl_protocol_vertex_array_element_t),
			.count=0xffffffff,
			.driver_handle=state->driver_handle
		};
		opengl_command_buffer_push_single(&(command.header));
		_gl_internal_state->gl_bound_vertex_array=state->header.index;
		goto _skip_vertex_array_sync;
	}
	opengl_protocol_update_vertex_array_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_UPDATE_VERTEX_ARRAY,
		.count=OPENGL_MAX_VERTEX_ATTRIBUTES,
		.driver_handle=state->driver_handle
	};
	state->stride=0;
	for (u32 i=0;i<OPENGL_MAX_VERTEX_ATTRIBUTES;i++){
		const opengl_vertex_array_state_entry_t* src_entry=state->entries+i+((state->enabled_entry_mask&(1<<i))?OPENGL_MAX_VERTEX_ATTRIBUTES:0);
		opengl_protocol_vertex_array_element_t* dst_entry=command.elements+i;
		dst_entry->index=i;
		dst_entry->size=(src_entry->size==GL_BGRA?OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_SIZE_BGRA:src_entry->size);
		switch (src_entry->type){
			case GL_BYTE:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_BYTE;
				break;
			case GL_UNSIGNED_BYTE:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_BYTE;
				break;
			case GL_SHORT:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_SHORT;
				break;
			case GL_UNSIGNED_SHORT:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_SHORT;
				break;
			case GL_INT:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT;
				break;
			case GL_UNSIGNED_INT:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT;
				break;
			case GL_HALF_FLOAT:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_HALF_FLOAT;
				break;
			case GL_FLOAT:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_FLOAT;
				break;
			case GL_DOUBLE:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_DOUBLE;
				break;
			case GL_INT_2_10_10_10_REV:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT_2_10_10_10_REV;
				break;
			case GL_UNSIGNED_INT_2_10_10_10_REV:
				dst_entry->type=OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT_2_10_10_10_REV;
				break;
		}
		if (state->enabled_entry_mask&(1<<i)){
			dst_entry->stride=src_entry->stride;
			dst_entry->divisor=src_entry->divisor;
		}
		else{
			dst_entry->stride=0;
			dst_entry->divisor=0;
		}
		dst_entry->offset=src_entry->offset;
		dst_entry->require_normalization=src_entry->normalized;
		src_entry=state->entries+i+OPENGL_MAX_VERTEX_ATTRIBUTES;
		u32 width=src_entry->size;
		switch (src_entry->type){
			case GL_SHORT:
			case GL_UNSIGNED_SHORT:
			case GL_HALF_FLOAT:
				width<<=1;
				break;
			case GL_INT:
			case GL_UNSIGNED_INT:
			case GL_FLOAT:
				width<<=2;
				break;
			case GL_DOUBLE:
				width<<=3;
				break;
			case GL_INT_2_10_10_10_REV:
			case GL_UNSIGNED_INT_2_10_10_10_REV:
				width=4;
				break;
		}
		if (src_entry->offset+width>state->stride){
			state->stride=src_entry->offset+width;
		}
	}
	command.header.length=sizeof(opengl_protocol_update_vertex_array_t)-(32-OPENGL_MAX_VERTEX_ATTRIBUTES)*sizeof(opengl_protocol_vertex_array_element_t);
	const opengl_protocol_update_vertex_array_t* output=(const opengl_protocol_update_vertex_array_t*)opengl_command_buffer_push_single(&(command.header));
	if (!state->driver_handle){
		opengl_command_buffer_flush();
		state->driver_handle=output->driver_handle;
	}
	state->needs_update=0;
_skip_vertex_array_sync:
	opengl_protocol_set_buffers_t set_buffers_command={
		.header.type=OPENGL_PROTOCOL_TYPE_SET_BUFFERS,
		.index_buffer_driver_handle=0,
		.index_buffer_index_width=0,
		.index_buffer_offset=0,
		.uniform_buffer_data=NULL,
		.uniform_buffer_size=0,
		.vertex_buffer_count=0,
		.vertex_buffers[0]={0,0,0}
	};
	_Bool update_buffers=0;
	if (_gl_internal_state->gl_bound_index_buffer!=_gl_internal_state->gl_used_index_buffer||_gl_internal_state->gl_bound_index_offset!=_gl_internal_state->gl_used_index_offset||_gl_internal_state->gl_bound_index_width!=_gl_internal_state->gl_used_index_width){
		opengl_buffer_state_t* index_buffer_state=_get_handle(_gl_internal_state->gl_used_index_buffer,OPENGL_HANDLE_TYPE_BUFFER,1);
		set_buffers_command.index_buffer_driver_handle=(index_buffer_state?index_buffer_state->driver_handle:0);
		set_buffers_command.index_buffer_index_width=_gl_internal_state->gl_used_index_width;
		set_buffers_command.index_buffer_offset=_gl_internal_state->gl_used_index_offset;
		_gl_internal_state->gl_bound_index_buffer=_gl_internal_state->gl_used_index_buffer;
		_gl_internal_state->gl_bound_index_offset=_gl_internal_state->gl_used_index_offset;
		_gl_internal_state->gl_bound_index_width=_gl_internal_state->gl_used_index_width;
		update_buffers=1;
	}
	if (_gl_internal_state->gl_constant_buffer_needs_update){
		opengl_program_state_t* program_state=_get_handle(_gl_internal_state->gl_used_program,OPENGL_HANDLE_TYPE_PROGRAM,1);
		set_buffers_command.uniform_buffer_data=(program_state?program_state->uniform_data:NULL);
		set_buffers_command.uniform_buffer_size=(program_state?program_state->uniform_data_size:0);
		_gl_internal_state->gl_constant_buffer_needs_update=0;
		update_buffers=1;
	}
	if (_gl_internal_state->gl_bound_array_buffer!=_gl_internal_state->gl_used_array_buffer){
		opengl_buffer_state_t* array_buffer_state=_get_handle(_gl_internal_state->gl_used_array_buffer,OPENGL_HANDLE_TYPE_BUFFER,1);
		opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
		if (!state){
			goto _skip_array_buffers_sync;
		}
		set_buffers_command.vertex_buffer_count=OPENGL_MAX_VERTEX_ATTRIBUTES;
		for (u32 i=0;i<OPENGL_MAX_VERTEX_ATTRIBUTES;i++){
			if (state->enabled_entry_mask&(1<<i)){
				(set_buffers_command.vertex_buffers+i)->driver_handle=(array_buffer_state?array_buffer_state->driver_handle:0);
				(set_buffers_command.vertex_buffers+i)->stride=state->stride;
				(set_buffers_command.vertex_buffers+i)->offset=0;
			}
			else{
				(set_buffers_command.vertex_buffers+i)->driver_handle=state->const_vertex_buffer_driver_handle;
				(set_buffers_command.vertex_buffers+i)->stride=0;
				(set_buffers_command.vertex_buffers+i)->offset=0;
			}
		}
		_gl_internal_state->gl_bound_array_buffer=_gl_internal_state->gl_used_array_buffer;
		update_buffers=1;
_skip_array_buffers_sync:
	}
	if (update_buffers){
		set_buffers_command.header.length=sizeof(opengl_protocol_set_buffers_t)-(32-set_buffers_command.vertex_buffer_count)*sizeof(opengl_protocol_vertex_buffer_config_t);
		opengl_command_buffer_push_single(&(set_buffers_command.header));
	}
	_gl_internal_state->gl_error=error;
}



static _Bool _process_draw_index_size(GLenum type){
	switch (type){
		case GL_UNSIGNED_BYTE:
			_gl_internal_state->gl_used_index_width=1;
			return 0;
		case GL_UNSIGNED_SHORT:
			_gl_internal_state->gl_used_index_width=2;
			return 0;
		case GL_UNSIGNED_INT:
			_gl_internal_state->gl_used_index_width=4;
			return 0;
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			return 1;
	}
}



static void _process_draw_command(GLenum mode,GLint start,GLsizei count,_Bool indexed,GLsizei instance_count,GLint index_bias,GLuint min_index,GLuint max_index){
	_sync_state();
	if (count<0||index_bias<0||max_index<min_index){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	opengl_protocol_draw_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_DRAW,
		.header.length=sizeof(opengl_protocol_draw_t),
		.start=start,
		.count=count,
		.indexed=indexed,
		.instance_count=instance_count,
		.index_bias=index_bias,
		.start_instance=0,
		.pritmitive_restart_index=_gl_internal_state->gl_primitive_restart_index,
		.min_index=min_index,
		.max_index=max_index
	};
	switch (mode){
		case GL_POINTS:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_POINTS;
			break;
		case GL_LINES:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_LINES;
			break;
		case GL_LINE_LOOP:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_LINE_LOOP;
			break;
		case GL_LINE_STRIP:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_LINE_STRIP;
			break;
		case GL_TRIANGLES:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_TRIANGLES;
			break;
		case GL_TRIANGLE_STRIP:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_STRIP;
			break;
		case GL_TRIANGLE_FAN:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_FAN;
			break;
		case GL_LINES_ADJACENCY:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_LINES_ADJACENCY;
			break;
		case GL_LINE_STRIP_ADJACENCY:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_LINE_STRIP_ADJACENCY;
			break;
		case GL_TRIANGLES_ADJACENCY:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_TRIANGLES_ADJACENCY;
			break;
		case GL_TRIANGLE_STRIP_ADJACENCY:
			command.mode=OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY;
			break;
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			return;
	}
	opengl_command_buffer_push_single(&(command.header));
	glFlush();
}



static void _update_buffer_data(GLenum target,GLintptr offset,GLsizeiptr size,const void* data,GLuint new_storage_type){
	if (offset<0||size<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	GLuint id=0;
	switch (target){
		case GL_ARRAY_BUFFER:
			id=_gl_internal_state->gl_used_array_buffer;
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			id=_gl_internal_state->gl_used_index_buffer;
			break;
		default:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: _update_buffer_data\x1b[0m\n");
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			return;
	}
	opengl_buffer_state_t* state=_get_handle(id,OPENGL_HANDLE_TYPE_BUFFER,1);
	if (!state){
		return;
	}
	if (state->is_mapped){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (new_storage_type==OPENGL_BUFFER_STORAGE_TYPE_NONE&&state->type==OPENGL_BUFFER_STORAGE_TYPE_NONE){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (new_storage_type!=OPENGL_BUFFER_STORAGE_TYPE_NONE){
		state->type=new_storage_type;
	}
	u32 type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_NO_CHANGE;
	if (new_storage_type==OPENGL_BUFFER_STORAGE_TYPE_STREAM){
		type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_STREAM;
	}
	else if (new_storage_type==OPENGL_BUFFER_STORAGE_TYPE_STATIC){
		type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_STATIC;
	}
	else if (new_storage_type==OPENGL_BUFFER_STORAGE_TYPE_DYNAMIC){
		type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_DYNAMIC;
	}
	opengl_protocol_update_buffer_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_UPDATE_BUFFER,
		.header.length=sizeof(opengl_protocol_update_buffer_t),
		.storage_type=type,
		.driver_handle=state->driver_handle,
		.offset=offset,
		.size=size,
		.data=data
	};
	const opengl_protocol_update_buffer_t* output=(const opengl_protocol_update_buffer_t*)opengl_command_buffer_push_single(&(command.header));
	glFlush();
	state->driver_handle=output->driver_handle;
}



static void _update_uniform(GLint location,const void* buffer,GLuint size){
	opengl_program_state_t* state=_get_handle(_gl_internal_state->gl_used_program,OPENGL_HANDLE_TYPE_PROGRAM,1);
	if (!state){
		return;
	}
	if (location<-1){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (location==-1){
		return;
	}
	location*=4*sizeof(float);
	if (location+size>state->uniform_data_size){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	sys_memory_copy(buffer,state->uniform_data+location,size);
	_gl_internal_state->gl_constant_buffer_needs_update=1;
}



static void _set_vertex_attrib_pointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer,GLboolean is_array){
	if (size!=GL_BGRA&&(size<1||size>4)){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (type!=GL_BYTE&&type!=GL_UNSIGNED_BYTE&&type!=GL_SHORT&&type!=GL_UNSIGNED_SHORT&&type!=GL_INT&&type!=GL_UNSIGNED_INT&&type!=GL_HALF_FLOAT&&type!=GL_FLOAT&&type!=GL_DOUBLE&&type!=GL_INT_2_10_10_10_REV&&type!=GL_UNSIGNED_INT_2_10_10_10_REV){
		_gl_internal_state->gl_error=GL_INVALID_ENUM;
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
	if (pointer&&!_gl_internal_state->gl_used_array_buffer){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (index>=OPENGL_MAX_VERTEX_ATTRIBUTES){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (stride<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		return;
	}
	(state->entries+index+is_array*OPENGL_MAX_VERTEX_ATTRIBUTES)->size=size;
	(state->entries+index+is_array*OPENGL_MAX_VERTEX_ATTRIBUTES)->type=type;
	(state->entries+index+is_array*OPENGL_MAX_VERTEX_ATTRIBUTES)->normalized=normalized;
	(state->entries+index+is_array*OPENGL_MAX_VERTEX_ATTRIBUTES)->stride=stride;
	(state->entries+index+is_array*OPENGL_MAX_VERTEX_ATTRIBUTES)->offset=(GLuint64)pointer;
	state->needs_update=1;
}



static void _set_vertex_attrib(GLuint index,GLint size,GLenum type,GLboolean normalized,const void* data,GLuint data_size){opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		return;
	}
	if (!state->const_vertex_buffer_driver_handle){
		opengl_protocol_update_buffer_t command={
			.header.type=OPENGL_PROTOCOL_TYPE_UPDATE_BUFFER,
			.header.length=sizeof(opengl_protocol_update_buffer_t),
			.storage_type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_STATIC,
			.driver_handle=0,
			.offset=0,
			.size=OPENGL_MAX_VERTEX_ATTRIBUTES*OPENGL_MAX_CONST_VERTEX_ELEMENT_SIZE,
			.data=NULL
		};
		const opengl_protocol_update_buffer_t* output=(const opengl_protocol_update_buffer_t*)opengl_command_buffer_push_single(&(command.header));
		glFlush();
		state->const_vertex_buffer_driver_handle=output->driver_handle;
	}
	opengl_protocol_update_buffer_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_UPDATE_BUFFER,
		.header.length=sizeof(opengl_protocol_update_buffer_t),
		.storage_type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_NO_CHANGE,
		.driver_handle=state->const_vertex_buffer_driver_handle,
		.offset=index*OPENGL_MAX_CONST_VERTEX_ELEMENT_SIZE,
		.size=data_size,
		.data=data
	};
	opengl_command_buffer_push_single(&(command.header));
	opengl_command_buffer_flush();
	_set_vertex_attrib_pointer(index,size,type,normalized,0,(void*)(u64)(index*OPENGL_MAX_CONST_VERTEX_ELEMENT_SIZE),0);
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
	opengl_program_state_t* state=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,0);
	if (!state){
		return;
	}
	opengl_shader_state_t* shader_state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER,0);
	if (!state){
		return;
	}
	if (shader_state->program){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	glsl_error_t error=glsl_linker_attach_program(&(state->linker_program),&(shader_state->compilation_output));
	if (error!=GLSL_NO_ERROR){
		sys_io_print("\x1b[1m\x1b[38;2;231;72;86mglAttachShader: %s\x1b[0m\n",error);
		glsl_error_delete(error);
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
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
	if (buffer&&!_get_handle(buffer,OPENGL_HANDLE_TYPE_BUFFER,1)){
		return;
	}
	switch (target){
		case GL_ARRAY_BUFFER:
			_gl_internal_state->gl_used_array_buffer=buffer;
			break;
		case GL_ELEMENT_ARRAY_BUFFER:
			_gl_internal_state->gl_used_index_buffer=buffer;
			break;
		default:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glBindBuffer\x1b[0m\n");
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			break;
	}
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
	if (array&&!_get_handle(array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1)){
		return;
	}
	_gl_internal_state->gl_used_vertex_array=array;
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
	GLuint new_storage_type;
	switch (usage){
		case GL_STREAM_DRAW:
		case GL_STREAM_READ:
		case GL_STREAM_COPY:
			new_storage_type=OPENGL_BUFFER_STORAGE_TYPE_STREAM;
			break;
		case GL_STATIC_DRAW:
		case GL_STATIC_READ:
		case GL_STATIC_COPY:
			new_storage_type=OPENGL_BUFFER_STORAGE_TYPE_STATIC;
			break;
		case GL_DYNAMIC_DRAW:
		case GL_DYNAMIC_READ:
		case GL_DYNAMIC_COPY:
			new_storage_type=OPENGL_BUFFER_STORAGE_TYPE_DYNAMIC;
			break;
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			return;
	}
	_update_buffer_data(target,0,size,data,new_storage_type);
}



SYS_PUBLIC void glBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,const void* data){
	_update_buffer_data(target,offset,size,data,OPENGL_BUFFER_STORAGE_TYPE_NONE);
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
	opengl_shader_state_t* state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER,0);
	if (!state){
		return;
	}
	state->was_compilation_attempted=1;
	if (state->error){
		glsl_error_delete(state->error);
		state->error=GLSL_NO_ERROR;
	}
	glsl_preprocessor_state_t preprocessor_state;
	glsl_preprocessor_state_init(&preprocessor_state);
	state->error=glsl_preprocessor_add_file((const char*)src_lib_opengl_rsrc_glsl_global_setup_glsl,0xffffffff,&preprocessor_state);
	if (state->error!=GLSL_NO_ERROR){
		return;
	}
	state->error=glsl_preprocessor_add_file((const char*)(state->type==GL_VERTEX_SHADER?src_lib_opengl_rsrc_glsl_vert_setup_glsl:src_lib_opengl_rsrc_glsl_frag_setup_glsl),0xffffffff,&preprocessor_state);
	if (state->error!=GLSL_NO_ERROR){
		return;
	}
	for (GLuint i=0;i<state->source_count;i++){
		state->error=glsl_preprocessor_add_file((state->sources+i)->data,i,&preprocessor_state);
		if (state->error!=GLSL_NO_ERROR){
			return;
		}
	}
	glsl_lexer_token_list_t token_list;
	state->error=glsl_lexer_extract_tokens(preprocessor_state.data,&token_list);
	glsl_preprocessor_state_delete(&preprocessor_state);
	if (state->error!=GLSL_NO_ERROR){
		return;
	}
	glsl_ast_t ast;
	state->error=glsl_parser_parse_tokens(&token_list,(state->type==GL_VERTEX_SHADER?GLSL_SHADER_TYPE_VERTEX:GLSL_SHADER_TYPE_FRAGMENT),&ast);
	glsl_lexer_delete_token_list(&token_list);
	if (state->error!=GLSL_NO_ERROR){
		return;
	}
	state->error=glsl_compiler_compile(&ast,&(state->compilation_output));
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
	glsl_linker_program_init(&(state->linker_program));
	state->was_linkage_attempted=0;
	state->error=GLSL_NO_ERROR;
	state->driver_handle=0;
	state->uniform_data=NULL;
	state->uniform_data_size=0;
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
	state->was_compilation_attempted=0;
	state->error=GLSL_NO_ERROR;
	state->program=NULL;
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
	if (index>=OPENGL_MAX_VERTEX_ATTRIBUTES){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		return;
	}
	state->enabled_entry_mask&=~(1<<index);
}



SYS_PUBLIC void glDrawArrays(GLenum mode,GLint first,GLsizei count){
	_process_draw_command(mode,first,count,0,0,0,first,count-first-1);
}



SYS_PUBLIC void glDrawArraysInstanced(GLenum mode,GLint first,GLsizei count,GLsizei instancecount){
	_process_draw_command(mode,first,count,0,instancecount,0,first,count-first-1);
}



SYS_PUBLIC void glDrawBuffer(GLenum buf){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawBuffer\x1b[0m\n");
}



SYS_PUBLIC void glDrawBuffers(GLsizei n,const GLenum* bufs){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glDrawBuffers\x1b[0m\n");
}



SYS_PUBLIC void glDrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices){
	if (_process_draw_index_size(type)){
		return;
	}
	_gl_internal_state->gl_used_index_offset=(GLuint64)indices;
	_process_draw_command(mode,0,count,1,0,0,0,count-1);
}



SYS_PUBLIC void glDrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	if (_process_draw_index_size(type)){
		return;
	}
	_gl_internal_state->gl_used_index_offset=(GLuint64)indices;
	_process_draw_command(mode,0,count,1,0,basevertex,0,count-1);
}



SYS_PUBLIC void glDrawElementsInstanced(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount){
	if (_process_draw_index_size(type)){
		return;
	}
	_gl_internal_state->gl_used_index_offset=(GLuint64)indices;
	_process_draw_command(mode,0,count,1,instancecount,0,0,count-1);
}



SYS_PUBLIC void glDrawElementsInstancedBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instancecount,GLint basevertex){
	if (_process_draw_index_size(type)){
		return;
	}
	_gl_internal_state->gl_used_index_offset=(GLuint64)indices;
	_process_draw_command(mode,0,count,1,instancecount,basevertex,0,count-1);
}



SYS_PUBLIC void glDrawRangeElements(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices){
	if (_process_draw_index_size(type)){
		return;
	}
	_gl_internal_state->gl_used_index_offset=(GLuint64)indices;
	_process_draw_command(mode,0,count,1,0,0,start,end);
}



SYS_PUBLIC void glDrawRangeElementsBaseVertex(GLenum mode,GLuint start,GLuint end,GLsizei count,GLenum type,const void* indices,GLint basevertex){
	if (_process_draw_index_size(type)){
		return;
	}
	_gl_internal_state->gl_used_index_offset=(GLuint64)indices;
	_process_draw_command(mode,0,count,1,basevertex,0,start,end);
}



SYS_PUBLIC void glEnable(GLenum cap){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEnable\x1b[0m\n");
}



SYS_PUBLIC void glEnablei(GLenum target,GLuint index){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glEnablei\x1b[0m\n");
}



SYS_PUBLIC void glEnableVertexAttribArray(GLuint index){
	if (index>=OPENGL_MAX_VERTEX_ATTRIBUTES){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		return;
	}
	state->enabled_entry_mask|=1<<index;
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
	opengl_protocol_header_t command={
		.type=OPENGL_PROTOCOL_TYPE_FLUSH,
		.length=sizeof(opengl_protocol_header_t)
	};
	opengl_command_buffer_push_single(&command);
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
	if (n<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	for (GLsizei i=0;i<n;i++){
		opengl_buffer_state_t* state=_alloc_handle(OPENGL_HANDLE_TYPE_BUFFER,sizeof(opengl_buffer_state_t));
		state->driver_handle=0;
		state->type=OPENGL_BUFFER_STORAGE_TYPE_NONE;
		state->is_mapped=0;
		state->size=0;
		buffers[i]=state->header.index;
	}
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
	if (n<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	for (GLsizei i=0;i<n;i++){
		opengl_vertex_array_state_t* state=_alloc_handle(OPENGL_HANDLE_TYPE_VERTEX_ARRAY,sizeof(opengl_vertex_array_state_t));
		state->enabled_entry_mask=0;
		for (GLuint j=0;j<OPENGL_MAX_VERTEX_ATTRIBUTES*2;j++){
			(state->entries+j)->size=0; // not 4 (as per standard) to prevent incorrect vertex buffer stride calculations
			(state->entries+j)->type=GL_FLOAT;
			(state->entries+j)->normalized=0;
			(state->entries+j)->divisor=0;
			(state->entries+j)->stride=0;
			(state->entries+j)->offset=0;
		}
		state->driver_handle=0;
		state->stride=0;
		state->needs_update=0;
		state->const_vertex_buffer_driver_handle=0;
		arrays[i]=state->header.index;
	}
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
	_get_parameter(target,index,data,OPENGL_PARAMETER_RETURN_TYPE_BOOL);
}



SYS_PUBLIC void glGetBooleanv(GLenum pname,GLboolean* data){
	_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_BOOL);
}



SYS_PUBLIC void glGetBufferParameteri64v(GLenum target,GLenum pname,GLint64* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferParameteri64v\x1b[0m\n");
}



SYS_PUBLIC void glGetBufferParameteriv(GLenum target,GLenum pname,GLint* params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferParameteriv\x1b[0m\n");
}



SYS_PUBLIC void glGetBufferPointerv(GLenum target,GLenum pname,void** params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferPointerv\x1b[0m\n");
}



SYS_PUBLIC void glGetBufferSubData(GLenum target,GLintptr offset,GLsizeiptr size,void* data){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetBufferSubData\x1b[0m\n");
}



SYS_PUBLIC void glGetCompressedTexImage(GLenum target,GLint level,void* img){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetCompressedTexImage\x1b[0m\n");
}



SYS_PUBLIC void glGetDoublev(GLenum pname,GLdouble* data){
	_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_DOUBLE);
}



SYS_PUBLIC GLenum glGetError(void){
	GLenum out=_gl_internal_state->gl_error;
	_gl_internal_state->gl_error=GL_NO_ERROR;
	return out;
}



SYS_PUBLIC void glGetFloatv(GLenum pname,GLfloat* data){
	_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_FLOAT);
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
	_get_parameter(target,index,data,OPENGL_PARAMETER_RETURN_TYPE_INT64);
}



SYS_PUBLIC void glGetInteger64v(GLenum pname,GLint64* data){
	_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_INT64);
}



SYS_PUBLIC void glGetIntegeri_v(GLenum target,GLuint index,GLint* data){
	_get_parameter(target,index,data,OPENGL_PARAMETER_RETURN_TYPE_INT);
}



SYS_PUBLIC void glGetIntegerv(GLenum pname,GLint* data){
	_get_parameter(pname,OPENGL_PARAMETER_NO_INDEX,data,OPENGL_PARAMETER_RETURN_TYPE_INT);
}



SYS_PUBLIC void glGetMultisamplefv(GLenum pname,GLuint index,GLfloat* val){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetMultisamplefv\x1b[0m\n");
}



SYS_PUBLIC void glGetPointerv(GLenum pname,void** params){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetPointerv\x1b[0m\n");
}



SYS_PUBLIC void glGetProgramInfoLog(GLuint program,GLsizei bufSize,GLsizei* length,GLchar* infoLog){
	opengl_program_state_t* state=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,0);
	if (!state){
		return;
	}
	if (bufSize<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (!bufSize){
		*length=0;
		return;
	}
	u32 size=(state->was_linkage_attempted&&state->error!=GLSL_NO_ERROR?sys_string_length(state->error):0);
	if (size>bufSize-1){
		size=bufSize-1;
	}
	sys_memory_copy(state->error,infoLog,size);
	infoLog[size]=0;
	*length=size;
}



SYS_PUBLIC void glGetProgramiv(GLuint program,GLenum pname,GLint* params){
	opengl_program_state_t* state=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,0);
	if (!state){
		return;
	}
	switch (pname){
		case GL_DELETE_STATUS:
			*params=0;
			break;
		case GL_LINK_STATUS:
			*params=(state->was_linkage_attempted&&state->error==GLSL_NO_ERROR);
			break;
		case GL_VALIDATE_STATUS:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_VALIDATE_STATUS\x1b[0m\n");
			*params=0;
			break;
		case GL_INFO_LOG_LENGTH:
			*params=(state->was_linkage_attempted&&state->error!=GLSL_NO_ERROR?sys_string_length(state->error):0);
			break;
		case GL_ATTACHED_SHADERS:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_ATTACHED_SHADERS\x1b[0m\n");
			*params=0;
			break;
		case GL_ACTIVE_ATTRIBUTES:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_ACTIVE_ATTRIBUTES\x1b[0m\n");
			*params=0;
			break;
		case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_ACTIVE_ATTRIBUTE_MAX_LENGTH\x1b[0m\n");
			*params=0;
			break;
		case GL_ACTIVE_UNIFORMS:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_ACTIVE_UNIFORMS\x1b[0m\n");
			*params=0;
			break;
		case GL_ACTIVE_UNIFORM_MAX_LENGTH:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_ACTIVE_UNIFORM_MAX_LENGTH\x1b[0m\n");
			*params=0;
			break;
		case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_TRANSFORM_FEEDBACK_BUFFER_MODE\x1b[0m\n");
			*params=0;
			break;
		case GL_TRANSFORM_FEEDBACK_VARYINGS:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_TRANSFORM_FEEDBACK_VARYINGS\x1b[0m\n");
			*params=0;
			break;
		case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH\x1b[0m\n");
			*params=0;
			break;
		case GL_GEOMETRY_VERTICES_OUT:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_GEOMETRY_VERTICES_OUT\x1b[0m\n");
			*params=0;
			break;
		case GL_GEOMETRY_INPUT_TYPE:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_GEOMETRY_INPUT_TYPE\x1b[0m\n");
			*params=0;
			break;
		case GL_GEOMETRY_OUTPUT_TYPE:
			sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetProgramiv: GL_GEOMETRY_OUTPUT_TYPE\x1b[0m\n");
			*params=0;
			break;
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			break;
	}
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
	opengl_shader_state_t* state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER,0);
	if (!state){
		return;
	}
	if (bufSize<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (!bufSize){
		*length=0;
		return;
	}
	u32 size=(state->was_compilation_attempted&&state->error!=GLSL_NO_ERROR?sys_string_length(state->error):0);
	if (size>bufSize-1){
		size=bufSize-1;
	}
	sys_memory_copy(state->error,infoLog,size);
	infoLog[size]=0;
	*length=size;
}



SYS_PUBLIC void glGetShaderiv(GLuint shader,GLenum pname,GLint* params){
	opengl_shader_state_t* state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER,0);
	if (!state){
		return;
	}
	switch (pname){
		case GL_SHADER_TYPE:
			*params=state->type;
			break;
		case GL_DELETE_STATUS:
			*params=0;
			break;
		case GL_COMPILE_STATUS:
			*params=(state->was_compilation_attempted&&state->error==GLSL_NO_ERROR);
			break;
		case GL_INFO_LOG_LENGTH:
			*params=(state->was_compilation_attempted&&state->error!=GLSL_NO_ERROR?sys_string_length(state->error):0);
			break;
		case GL_SHADER_SOURCE_LENGTH:
			GLint out=!!state->source_count;
			for (GLuint i=0;i<state->source_count;i++){
				out+=(state->sources+i)->length;
			}
			*params=out;
			break;
		default:
			_gl_internal_state->gl_error=GL_INVALID_ENUM;
			break;
	}
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
	if (!program){
		_gl_internal_state->gl_used_program=0;
		return 0;
	}
	opengl_program_state_t* state=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,0);
	if (!state){
		return 0;
	}
	if (!state->was_linkage_attempted||state->error){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return 0;
	}
	for (u32 i=0;i<state->linked_program.uniform_count;i++){
		if (!sys_string_compare((state->linked_program.uniforms+i)->name,name)){
			return (state->linked_program.uniforms+i)->slot;
		}
	}
	return -1;
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



SYS_PUBLIC void glGetVertexAttribPointerv(GLuint index,GLenum pname,void** pointer){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glGetVertexAttribPointerv\x1b[0m\n");
}



SYS_PUBLIC void glHint(GLenum target,GLenum mode){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glHint\x1b[0m\n");
}



SYS_PUBLIC GLboolean glIsBuffer(GLuint buffer){
	GLenum error=_gl_internal_state->gl_error;
	void* ptr=_get_handle(buffer,OPENGL_HANDLE_TYPE_BUFFER,1);
	_gl_internal_state->gl_error=error;
	return !!ptr;
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
	GLenum error=_gl_internal_state->gl_error;
	void* ptr=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,1);
	_gl_internal_state->gl_error=error;
	return !!ptr;
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
	GLenum error=_gl_internal_state->gl_error;
	void* ptr=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER,1);
	_gl_internal_state->gl_error=error;
	return !!ptr;
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
	GLenum error=_gl_internal_state->gl_error;
	void* ptr=_get_handle(array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	_gl_internal_state->gl_error=error;
	return !!ptr;
}



SYS_PUBLIC void glLineWidth(GLfloat width){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glLineWidth\x1b[0m\n");
}



SYS_PUBLIC void glLinkProgram(GLuint program){
	opengl_program_state_t* state=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,0);
	if (!state){
		return;
	}
	state->was_linkage_attempted=1;
	if (state->error){
		glsl_error_delete(state->error);
		state->error=GLSL_NO_ERROR;
	}
	state->error=glsl_linker_program_link(&(state->linker_program),_gl_internal_state->glsl_backend_descriptor,&(state->linked_program));
	if (state->error){
		return;
	}
	state->uniform_data_size=state->linked_program.uniform_slot_count*4*sizeof(float);
	state->uniform_data=sys_heap_alloc(NULL,state->uniform_data_size);
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
	_gl_internal_state->gl_primitive_restart_index=index;
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
	opengl_shader_state_t* state=_get_handle(shader,OPENGL_HANDLE_TYPE_SHADER,0);
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
	_update_uniform(location,&v0,sizeof(GLfloat));
}



SYS_PUBLIC void glUniform1fv(GLint location,GLsizei count,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform1i(GLint location,GLint v0){
	_update_uniform(location,&v0,sizeof(GLint));
}



SYS_PUBLIC void glUniform1iv(GLint location,GLsizei count,const GLint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*sizeof(GLint));
}



SYS_PUBLIC void glUniform1ui(GLint location,GLuint v0){
	_update_uniform(location,&v0,sizeof(GLuint));
}



SYS_PUBLIC void glUniform1uiv(GLint location,GLsizei count,const GLuint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*sizeof(GLuint));
}



SYS_PUBLIC void glUniform2f(GLint location,GLfloat v0,GLfloat v1){
	GLfloat buffer[2]={v0,v1};
	_update_uniform(location,buffer,2*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform2fv(GLint location,GLsizei count,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*2*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform2i(GLint location,GLint v0,GLint v1){
	GLint buffer[2]={v0,v1};
	_update_uniform(location,buffer,2*sizeof(GLint));
}



SYS_PUBLIC void glUniform2iv(GLint location,GLsizei count,const GLint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*2*sizeof(GLint));
}



SYS_PUBLIC void glUniform2ui(GLint location,GLuint v0,GLuint v1){
	GLuint buffer[2]={v0,v1};
	_update_uniform(location,buffer,2*sizeof(GLuint));
}



SYS_PUBLIC void glUniform2uiv(GLint location,GLsizei count,const GLuint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*2*sizeof(GLuint));
}



SYS_PUBLIC void glUniform3f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2){
	GLfloat buffer[3]={v0,v1,v2};
	_update_uniform(location,buffer,3*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform3fv(GLint location,GLsizei count,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*3*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform3i(GLint location,GLint v0,GLint v1,GLint v2){
	GLint buffer[3]={v0,v1,v2};
	_update_uniform(location,buffer,3*sizeof(GLint));
}



SYS_PUBLIC void glUniform3iv(GLint location,GLsizei count,const GLint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*3*sizeof(GLint));
}



SYS_PUBLIC void glUniform3ui(GLint location,GLuint v0,GLuint v1,GLuint v2){
	GLuint buffer[3]={v0,v1,v2};
	_update_uniform(location,buffer,3*sizeof(GLuint));
}



SYS_PUBLIC void glUniform3uiv(GLint location,GLsizei count,const GLuint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*3*sizeof(GLuint));
}



SYS_PUBLIC void glUniform4f(GLint location,GLfloat v0,GLfloat v1,GLfloat v2,GLfloat v3){
	GLfloat buffer[4]={v0,v1,v2,v3};
	_update_uniform(location,buffer,4*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform4fv(GLint location,GLsizei count,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*4*sizeof(GLfloat));
}



SYS_PUBLIC void glUniform4i(GLint location,GLint v0,GLint v1,GLint v2,GLint v3){
	GLint buffer[4]={v0,v1,v2,v3};
	_update_uniform(location,buffer,4*sizeof(GLint));
}



SYS_PUBLIC void glUniform4iv(GLint location,GLsizei count,const GLint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*4*sizeof(GLint));
}



SYS_PUBLIC void glUniform4ui(GLint location,GLuint v0,GLuint v1,GLuint v2,GLuint v3){
	GLuint buffer[4]={v0,v1,v2,v3};
	_update_uniform(location,buffer,4*sizeof(GLuint));
}



SYS_PUBLIC void glUniform4uiv(GLint location,GLsizei count,const GLuint* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	_update_uniform(location,value,count*4*sizeof(GLuint));
}



SYS_PUBLIC void glUniformBlockBinding(GLuint program,GLuint uniformBlockIndex,GLuint uniformBlockBinding){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUniformBlockBinding\x1b[0m\n");
}



SYS_PUBLIC void glUniformMatrix2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*2;i++){
			_update_uniform(location+i,value+i*2,2*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[4]={
			value[0],value[2],
			value[1],value[3]
		};
		_update_uniform(location+i*2,buffer,2*sizeof(GLfloat));
		_update_uniform(location+i*2+1,buffer+2,2*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix2x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*3;i++){
			_update_uniform(location+i,value+i*2,2*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[6]={
			value[0],value[2],value[4],
			value[1],value[3],value[5]
		};
		_update_uniform(location+i*2,buffer,3*sizeof(GLfloat));
		_update_uniform(location+i*2+1,buffer+3,3*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix2x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*4;i++){
			_update_uniform(location+i,value+i*2,2*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[8]={
			value[0],value[2],value[4],value[6],
			value[1],value[3],value[5],value[7]
		};
		_update_uniform(location+i*2,buffer,4*sizeof(GLfloat));
		_update_uniform(location+i*2+1,buffer+4,4*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*3;i++){
			_update_uniform(location+i,value+i*3,3*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[9]={
			value[0],value[3],value[6],
			value[1],value[4],value[7],
			value[2],value[5],value[8]
		};
		_update_uniform(location+i*3,buffer,3*sizeof(GLfloat));
		_update_uniform(location+i*3+1,buffer+3,3*sizeof(GLfloat));
		_update_uniform(location+i*3+2,buffer+6,3*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix3x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*2;i++){
			_update_uniform(location+i,value+i*3,3*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[6]={
			value[0],value[3],
			value[1],value[4],
			value[2],value[5]
		};
		_update_uniform(location+i*3,buffer,2*sizeof(GLfloat));
		_update_uniform(location+i*3+1,buffer+2,2*sizeof(GLfloat));
		_update_uniform(location+i*3+2,buffer+4,2*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix3x4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*4;i++){
			_update_uniform(location+i,value+i*3,3*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[12]={
			value[0],value[3],value[6],value[9],
			value[1],value[4],value[7],value[10],
			value[2],value[5],value[8],value[11]
		};
		_update_uniform(location+i*3,buffer,4*sizeof(GLfloat));
		_update_uniform(location+i*3+1,buffer+4,4*sizeof(GLfloat));
		_update_uniform(location+i*3+2,buffer+8,4*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix4fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*4;i++){
			_update_uniform(location+i,value+i*4,4*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[16]={
			value[0],value[4],value[8],value[12],
			value[1],value[5],value[9],value[13],
			value[2],value[6],value[10],value[14],
			value[3],value[7],value[11],value[15]
		};
		_update_uniform(location+i*4,buffer,4*sizeof(GLfloat));
		_update_uniform(location+i*4+1,buffer+4,4*sizeof(GLfloat));
		_update_uniform(location+i*4+2,buffer+8,4*sizeof(GLfloat));
		_update_uniform(location+i*4+3,buffer+12,4*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix4x2fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*2;i++){
			_update_uniform(location+i,value+i*4,4*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[8]={
			value[0],value[4],
			value[1],value[5],
			value[2],value[6],
			value[3],value[7]
		};
		_update_uniform(location+i*4,buffer,2*sizeof(GLfloat));
		_update_uniform(location+i*4+1,buffer+2,2*sizeof(GLfloat));
		_update_uniform(location+i*4+2,buffer+4,2*sizeof(GLfloat));
		_update_uniform(location+i*4+3,buffer+6,2*sizeof(GLfloat));
	}
}



SYS_PUBLIC void glUniformMatrix4x3fv(GLint location,GLsizei count,GLboolean transpose,const GLfloat* value){
	if (count<0){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (!transpose){
		for (GLsizei i=0;i<count*3;i++){
			_update_uniform(location+i,value+i*4,4*sizeof(GLfloat));
		}
		return;
	}
	for (GLsizei i=0;i<count;i++){
		GLfloat buffer[12]={
			value[0],value[4],value[8],
			value[1],value[5],value[9],
			value[2],value[6],value[10],
			value[3],value[7],value[11]
		};
		_update_uniform(location+i*4,buffer,3*sizeof(GLfloat));
		_update_uniform(location+i*4+1,buffer+3,3*sizeof(GLfloat));
		_update_uniform(location+i*4+2,buffer+6,3*sizeof(GLfloat));
		_update_uniform(location+i*4+3,buffer+9,3*sizeof(GLfloat));
	}
}



SYS_PUBLIC GLboolean glUnmapBuffer(GLenum target){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glUnmapBuffer\x1b[0m\n");
	return 0;
}



SYS_PUBLIC void glUseProgram(GLuint program){
	if (!program){
		_gl_internal_state->gl_used_program=0;
		_gl_internal_state->gl_constant_buffer_needs_update=1;
		return;
	}
	opengl_program_state_t* state=_get_handle(program,OPENGL_HANDLE_TYPE_PROGRAM,0);
	if (!state){
		return;
	}
	if (!state->driver_handle){
		opengl_protocol_create_shader_t command={
			.header.type=OPENGL_PROTOCOL_TYPE_CREATE_SHADER,
			.header.length=sizeof(opengl_protocol_create_shader_t),
			.format=OPENGL_PROTOCOL_SHADER_FORMAT_TGSI,
			.vertex_shader_size=(state->linked_program.shaders+GLSL_SHADER_TYPE_VERTEX)->length,
			.fragment_shader_size=(state->linked_program.shaders+GLSL_SHADER_TYPE_FRAGMENT)->length,
			.vertex_shader_data=(state->linked_program.shaders+GLSL_SHADER_TYPE_VERTEX)->data,
			.fragment_shader_data=(state->linked_program.shaders+GLSL_SHADER_TYPE_FRAGMENT)->data
		};
		const opengl_protocol_create_shader_t* output=(const opengl_protocol_create_shader_t*)opengl_command_buffer_push_single(&(command.header));
		glFlush();
		state->driver_handle=output->driver_handle;
	}
	_gl_internal_state->gl_used_program=state->header.index;
	opengl_protocol_use_shader_t command={
		.header.type=OPENGL_PROTOCOL_TYPE_USE_SHADER,
		.header.length=sizeof(opengl_protocol_use_shader_t),
		.driver_handle=state->driver_handle
	};
	opengl_command_buffer_push_single(&(command.header));
	_gl_internal_state->gl_constant_buffer_needs_update=1;
}



SYS_PUBLIC void glValidateProgram(GLuint program){
	sys_io_print("\x1b[1m\x1b[38;2;231;72;86mUnimplemented: glValidateProgram\x1b[0m\n");
}



SYS_PUBLIC void glVertexAttrib1d(GLuint index,GLdouble x){
	_set_vertex_attrib(index,1,GL_DOUBLE,0,&x,sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib1dv(GLuint index,const GLdouble* v){
	_set_vertex_attrib(index,1,GL_DOUBLE,0,v,sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib1f(GLuint index,GLfloat x){
	_set_vertex_attrib(index,1,GL_FLOAT,0,&x,sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib1fv(GLuint index,const GLfloat* v){
	_set_vertex_attrib(index,1,GL_FLOAT,0,v,sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib1s(GLuint index,GLshort x){
	_set_vertex_attrib(index,1,GL_SHORT,0,&x,sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib1sv(GLuint index,const GLshort* v){
	_set_vertex_attrib(index,1,GL_SHORT,0,v,sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib2d(GLuint index,GLdouble x,GLdouble y){
	GLdouble buffer[2]={x,y};
	_set_vertex_attrib(index,2,GL_DOUBLE,0,buffer,2*sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib2dv(GLuint index,const GLdouble* v){
	_set_vertex_attrib(index,2,GL_DOUBLE,0,v,2*sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib2f(GLuint index,GLfloat x,GLfloat y){
	GLfloat buffer[2]={x,y};
	_set_vertex_attrib(index,2,GL_FLOAT,0,buffer,2*sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib2fv(GLuint index,const GLfloat* v){
	_set_vertex_attrib(index,2,GL_FLOAT,0,v,2*sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib2s(GLuint index,GLshort x,GLshort y){
	GLshort buffer[2]={x,y};
	_set_vertex_attrib(index,2,GL_SHORT,0,buffer,2*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib2sv(GLuint index,const GLshort* v){
	_set_vertex_attrib(index,2,GL_SHORT,0,v,2*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib3d(GLuint index,GLdouble x,GLdouble y,GLdouble z){
	GLdouble buffer[3]={x,y,z};
	_set_vertex_attrib(index,3,GL_DOUBLE,0,buffer,3*sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib3dv(GLuint index,const GLdouble* v){
	_set_vertex_attrib(index,3,GL_DOUBLE,0,v,3*sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib3f(GLuint index,GLfloat x,GLfloat y,GLfloat z){
	GLfloat buffer[3]={x,y,z};
	_set_vertex_attrib(index,3,GL_FLOAT,0,buffer,3*sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib3fv(GLuint index,const GLfloat* v){
	_set_vertex_attrib(index,3,GL_FLOAT,0,v,3*sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib3s(GLuint index,GLshort x,GLshort y,GLshort z){
	GLshort buffer[3]={x,y,z};
	_set_vertex_attrib(index,3,GL_SHORT,0,buffer,3*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib3sv(GLuint index,const GLshort* v){
	_set_vertex_attrib(index,3,GL_SHORT,0,v,3*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib4bv(GLuint index,const GLbyte* v){
	_set_vertex_attrib(index,4,GL_SHORT,0,v,4*sizeof(GLbyte));
}



SYS_PUBLIC void glVertexAttrib4d(GLuint index,GLdouble x,GLdouble y,GLdouble z,GLdouble w){
	GLdouble buffer[4]={x,y,z,w};
	_set_vertex_attrib(index,4,GL_DOUBLE,0,buffer,4*sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib4dv(GLuint index,const GLdouble* v){
	_set_vertex_attrib(index,4,GL_DOUBLE,0,v,4*sizeof(GLdouble));
}



SYS_PUBLIC void glVertexAttrib4f(GLuint index,GLfloat x,GLfloat y,GLfloat z,GLfloat w){
	GLfloat buffer[4]={x,y,z,w};
	_set_vertex_attrib(index,4,GL_FLOAT,0,buffer,4*sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib4fv(GLuint index,const GLfloat* v){
	_set_vertex_attrib(index,4,GL_FLOAT,0,v,4*sizeof(GLfloat));
}



SYS_PUBLIC void glVertexAttrib4iv(GLuint index,const GLint* v){
	_set_vertex_attrib(index,4,GL_INT,0,v,4*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttrib4Nbv(GLuint index,const GLbyte* v){
	_set_vertex_attrib(index,4,GL_BYTE,1,v,4*sizeof(GLbyte));
}



SYS_PUBLIC void glVertexAttrib4Niv(GLuint index,const GLint* v){
	_set_vertex_attrib(index,4,GL_INT,1,v,4*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttrib4Nsv(GLuint index,const GLshort* v){
	_set_vertex_attrib(index,4,GL_SHORT,1,v,4*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib4Nub(GLuint index,GLubyte x,GLubyte y,GLubyte z,GLubyte w){
	GLubyte buffer[4]={x,y,z,w};
	_set_vertex_attrib(index,4,GL_UNSIGNED_BYTE,1,buffer,4*sizeof(GLubyte));
}



SYS_PUBLIC void glVertexAttrib4Nubv(GLuint index,const GLubyte* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_BYTE,1,v,4*sizeof(GLubyte));
}



SYS_PUBLIC void glVertexAttrib4Nuiv(GLuint index,const GLuint* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_INT,1,v,4*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttrib4Nusv(GLuint index,const GLushort* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_SHORT,1,v,4*sizeof(GLushort));
}



SYS_PUBLIC void glVertexAttrib4s(GLuint index,GLshort x,GLshort y,GLshort z,GLshort w){
	GLshort buffer[4]={x,y,z,w};
	_set_vertex_attrib(index,4,GL_SHORT,0,buffer,4*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib4sv(GLuint index,const GLshort* v){
	_set_vertex_attrib(index,4,GL_SHORT,0,v,4*sizeof(GLshort));
}



SYS_PUBLIC void glVertexAttrib4ubv(GLuint index,const GLubyte* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_BYTE,0,v,4*sizeof(GLubyte));
}



SYS_PUBLIC void glVertexAttrib4uiv(GLuint index,const GLuint* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_INT,0,v,4*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttrib4usv(GLuint index,const GLushort* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_SHORT,0,v,4*sizeof(GLushort));
}



SYS_PUBLIC void glVertexAttribDivisor(GLuint index,GLuint divisor){
	if (index>=OPENGL_MAX_VERTEX_ATTRIBUTES){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		return;
	}
	(state->entries+index+OPENGL_MAX_VERTEX_ATTRIBUTES)->divisor=divisor;
	state->needs_update=1;
}



SYS_PUBLIC void glVertexAttribI1i(GLuint index,GLint x){
	_set_vertex_attrib(index,1,GL_INT,0,&x,1*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI1iv(GLuint index,const GLint* v){
	_set_vertex_attrib(index,1,GL_INT,0,v,1*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI1ui(GLuint index,GLuint x){
	_set_vertex_attrib(index,1,GL_UNSIGNED_INT,0,&x,1*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI1uiv(GLuint index,const GLuint* v){
	_set_vertex_attrib(index,1,GL_UNSIGNED_INT,0,v,1*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI2i(GLuint index,GLint x,GLint y){
	GLint buffer[2]={x,y};
	_set_vertex_attrib(index,2,GL_INT,0,buffer,2*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI2iv(GLuint index,const GLint* v){
	_set_vertex_attrib(index,2,GL_UNSIGNED_INT,0,v,2*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI2ui(GLuint index,GLuint x,GLuint y){
	GLuint buffer[2]={x,y};
	_set_vertex_attrib(index,2,GL_UNSIGNED_INT,0,buffer,2*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI2uiv(GLuint index,const GLuint* v){
	_set_vertex_attrib(index,2,GL_UNSIGNED_INT,0,v,2*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI3i(GLuint index,GLint x,GLint y,GLint z){
	GLint buffer[3]={x,y,z};
	_set_vertex_attrib(index,3,GL_INT,0,buffer,3*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI3iv(GLuint index,const GLint* v){
	_set_vertex_attrib(index,3,GL_INT,0,v,3*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI3ui(GLuint index,GLuint x,GLuint y,GLuint z){
	GLuint buffer[3]={x,y,z};
	_set_vertex_attrib(index,3,GL_UNSIGNED_INT,0,buffer,3*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI3uiv(GLuint index,const GLuint* v){
	_set_vertex_attrib(index,3,GL_UNSIGNED_INT,0,v,3*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI4bv(GLuint index,const GLbyte* v){
	GLint buffer[4]={v[0],v[1],v[2],v[3]};
	_set_vertex_attrib(index,4,GL_INT,0,buffer,4*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI4i(GLuint index,GLint x,GLint y,GLint z,GLint w){
	GLint buffer[4]={x,y,z,w};
	_set_vertex_attrib(index,4,GL_INT,0,buffer,4*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI4iv(GLuint index,const GLint* v){
	_set_vertex_attrib(index,4,GL_INT,0,v,4*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI4sv(GLuint index,const GLshort* v){
	GLint buffer[4]={v[0],v[1],v[2],v[3]};
	_set_vertex_attrib(index,4,GL_INT,0,buffer,4*sizeof(GLint));
}



SYS_PUBLIC void glVertexAttribI4ubv(GLuint index,const GLubyte* v){
	GLuint buffer[4]={v[0],v[1],v[2],v[3]};
	_set_vertex_attrib(index,4,GL_UNSIGNED_INT,0,buffer,4*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI4ui(GLuint index,GLuint x,GLuint y,GLuint z,GLuint w){
	GLuint buffer[4]={x,y,z,w};
	_set_vertex_attrib(index,4,GL_UNSIGNED_INT,0,buffer,4*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI4uiv(GLuint index,const GLuint* v){
	_set_vertex_attrib(index,4,GL_UNSIGNED_INT,0,v,4*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribI4usv(GLuint index,const GLushort* v){
	GLuint buffer[4]={v[0],v[1],v[2],v[3]};
	_set_vertex_attrib(index,4,GL_UNSIGNED_INT,0,buffer,4*sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribIPointer(GLuint index,GLint size,GLenum type,GLsizei stride,const void* pointer){
	if (size<1||size>4){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (type!=GL_BYTE&&type!=GL_UNSIGNED_BYTE&&type!=GL_SHORT&&type!=GL_UNSIGNED_SHORT&&type!=GL_INT&&type!=GL_UNSIGNED_INT){
		_gl_internal_state->gl_error=GL_INVALID_ENUM;
		return;
	}
	if (pointer&&!_gl_internal_state->gl_used_array_buffer){
		_gl_internal_state->gl_error=GL_INVALID_OPERATION;
		return;
	}
	if (index>=OPENGL_MAX_VERTEX_ATTRIBUTES){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	if (stride<0){
		_gl_internal_state->gl_error=GL_INVALID_VALUE;
		return;
	}
	opengl_vertex_array_state_t* state=_get_handle(_gl_internal_state->gl_used_vertex_array,OPENGL_HANDLE_TYPE_VERTEX_ARRAY,1);
	if (!state){
		return;
	}
	(state->entries+index+OPENGL_MAX_VERTEX_ATTRIBUTES)->size=size;
	(state->entries+index+OPENGL_MAX_VERTEX_ATTRIBUTES)->type=type;
	(state->entries+index+OPENGL_MAX_VERTEX_ATTRIBUTES)->normalized=0;
	(state->entries+index+OPENGL_MAX_VERTEX_ATTRIBUTES)->stride=stride;
	(state->entries+index+OPENGL_MAX_VERTEX_ATTRIBUTES)->offset=(GLuint64)pointer;
	state->needs_update=1;
}



SYS_PUBLIC void glVertexAttribP1ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	_set_vertex_attrib(index,4,type,normalized,&value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP1uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	_set_vertex_attrib(index,4,type,normalized,value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP2ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	_set_vertex_attrib(index,4,type,normalized,&value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP2uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	_set_vertex_attrib(index,4,type,normalized,value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP3ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	_set_vertex_attrib(index,4,type,normalized,&value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP3uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	_set_vertex_attrib(index,4,type,normalized,value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP4ui(GLuint index,GLenum type,GLboolean normalized,GLuint value){
	_set_vertex_attrib(index,4,type,normalized,&value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribP4uiv(GLuint index,GLenum type,GLboolean normalized,const GLuint* value){
	_set_vertex_attrib(index,4,type,normalized,value,sizeof(GLuint));
}



SYS_PUBLIC void glVertexAttribPointer(GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const void* pointer){
	_set_vertex_attrib_pointer(index,size,type,normalized,stride,pointer,1);
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
