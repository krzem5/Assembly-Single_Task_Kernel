#include <glsl/ast.h>
#include <glsl/backend.h>
#include <glsl/compiler.h>
#include <glsl/debug.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>
#include <glsl/parser.h>
#include <glsl/preprocessor.h>
#include <sys/io/io.h>
#include <sys/lib/lib.h>
#include <sys/util/options.h>



#define BACKEND_LIB_NAME "libtgsiascii.so"



static const char global_setup[]=" \
const int gl_MaxVertexAttribs=16; \n\
const int gl_MaxVertexUniformComponents=1024; \n\
const int gl_MaxVaryingFloats=60; \n\
const int gl_MaxVaryingComponents=60; \n\
const int gl_MaxVertexOutputComponents=64; \n\
const int gl_MaxGeometryInputComponents=64; \n\
const int gl_MaxGeometryOutputComponents=128; \n\
const int gl_MaxFragmentInputComponents=128; \n\
const int gl_MaxVertexTextureImageUnits=16; \n\
const int gl_MaxCombinedTextureImageUnits=48; \n\
const int gl_MaxTextureImageUnits=16; \n\
const int gl_MaxFragmentUniformComponents=1024; \n\
const int gl_MaxDrawBuffers=8; \n\
const int gl_MaxClipDistances=8; \n\
const int gl_MaxGeometryTextureImageUnits=16; \n\
const int gl_MaxGeometryOutputVertices=256; \n\
const int gl_MaxGeometryTotalOutputComponents=1024; \n\
const int gl_MaxGeometryUniformComponents=1024; \n\
const int gl_MaxGeometryVaryingComponents=64; \n\
";



static const char vertex_shader_setup[]=" \
#version 330 core \n\
#define __VERSION__ 330 \n\
#define GL_core_profile 1 \n\
 \n\
 \n\
 \n\
in int gl_VertexID; \n\
in int gl_InstanceID; \n\
out gl_PerVertex{ \n\
	vec4 gl_Position; \n\
	float gl_PointSize; \n\
	float gl_ClipDistance[]; \n\
}; \n\
";



static const char fragment_shader_setup[]=" \
#version 330 core \n\
#define __VERSION__ 330 \n\
#define GL_core_profile 1 \n\
 \n\
 \n\
 \n\
in vec4 gl_FragCoord; \n\
in bool gl_FrontFacing; \n\
in float gl_ClipDistance[]; \n\
out float gl_FragDepth; \n\
in vec2 gl_PointCoord; \n\
in int gl_PrimitiveID; \n\
";



static const char vertex_shader[]=" \
#version 330 core \n\
 \n\
 \n\
 \n\
layout (location=0) in vec2 in_pos; \n\
layout (location=1) in vec3 in_color; \n\
out vec3 fs_color; \n\
uniform mat3 vs_transform; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	vec3 projection=vs_transform*vec3(in_pos,1.0).xyz; \n\
	gl_Position=vec4(projection.xy,0.0,1.0); \n\
	fs_color=in_color*vec3(1.0,1.0,1.0-2.0*projection.z)+vec3(0.0,0.0,projection.z); \n\
} \n\
";



static const char fragment_shader[]=" \
#version 330 core \n\
 \n\
 \n\
 \n\
in vec3 fs_color; \n\
out vec4 out_color; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	out_color=vec4(fs_color,1.0); \n\
} \n\
";



int main(int argc,const char** argv){
	if (!sys_options_parse_NEW(argc,argv,"")){
		return 1;
	}
	sys_library_t backend_lib=sys_lib_load(BACKEND_LIB_NAME,SYS_LIB_LOAD_FLAG_RESOLVE_SYMBOLS);
	if (SYS_IS_ERROR(backend_lib)){
		sys_io_print("Unable to open backend library '%s'\n",BACKEND_LIB_NAME);
		return 1;
	}
	glsl_preprocessor_state_t preprocessor_state;
	glsl_preprocessor_state_init(&preprocessor_state);
	glsl_error_t error=glsl_preprocessor_add_file(&preprocessor_state,global_setup,0xffffffff);
	if (error){
		goto _error;
	}
	error=glsl_preprocessor_add_file(&preprocessor_state,vertex_shader_setup,0xffffffff);
	if (error){
		goto _error;
	}
	error=glsl_preprocessor_add_file(&preprocessor_state,vertex_shader,0);
	if (error){
		goto _error;
	}
	// sys_io_print("%s\n",preprocessor_state.data);
	glsl_lexer_token_list_t token_list;
	error=glsl_lexer_extract_tokens(preprocessor_state.data,&token_list);
	glsl_preprocessor_state_deinit(&preprocessor_state);
	if (error){
		goto _error;
	}
	// glsl_debug_print_token_list(&token_list);
	glsl_ast_t ast;
	error=glsl_parser_parse_tokens(&token_list,GLSL_SHADER_TYPE_VERTEX,&ast);
	glsl_lexer_delete_token_list(&token_list);
	if (error){
		goto _error;
	}
	// glsl_debug_print_ast(&ast);
	glsl_compilation_output_t compilation_output;
	error=glsl_compiler_compile(&ast,&compilation_output);
	if (error){
		goto _error;
	}
	// glsl_debug_print_compilation_output(&compilation_output);
	glsl_linker_program_t program;
	glsl_linker_program_init(&program);
	error=glsl_linker_attach_program(&program,&compilation_output);
	if (error){
		goto _error;
	}
	glsl_preprocessor_state_init(&preprocessor_state);
	error=glsl_preprocessor_add_file(&preprocessor_state,global_setup,0xffffffff);
	if (error){
		goto _error;
	}
	error=glsl_preprocessor_add_file(&preprocessor_state,fragment_shader_setup,0xffffffff);
	if (error){
		goto _error;
	}
	error=glsl_preprocessor_add_file(&preprocessor_state,fragment_shader,0);
	if (error){
		goto _error;
	}
	// sys_io_print("%s\n",preprocessor_state.data);
	error=glsl_lexer_extract_tokens(preprocessor_state.data,&token_list);
	glsl_preprocessor_state_deinit(&preprocessor_state);
	if (error){
		goto _error;
	}
	// glsl_debug_print_token_list(&token_list);
	error=glsl_parser_parse_tokens(&token_list,GLSL_SHADER_TYPE_FRAGMENT,&ast);
	glsl_lexer_delete_token_list(&token_list);
	if (error){
		goto _error;
	}
	// glsl_debug_print_ast(&ast);
	error=glsl_compiler_compile(&ast,&compilation_output);
	if (error){
		goto _error;
	}
	// glsl_debug_print_compilation_output(&compilation_output);
	error=glsl_linker_attach_program(&program,&compilation_output);
	if (error){
		goto _error;
	}
	glsl_linker_linked_program_t linked_program;
	error=glsl_linker_program_link(&program,((glsl_backend_descriptor_query_func_t)sys_lib_lookup_symbol(backend_lib,"_glsl_backend_query_descriptor"))(),&linked_program);
	glsl_linker_program_delete(&program);
	if (error){
		goto _error;
	}
	glsl_debug_print_linked_program(&linked_program);
	glsl_linker_linked_program_delete(&linked_program);
	return 0;
_error:
	sys_io_print("%s\n",error);
	glsl_error_delete(error);
	return 1;
}
