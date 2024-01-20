#include <glsl/ast.h>
#include <glsl/debug.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>
#include <glsl/parser.h>
#include <glsl/preprocessor.h>
#include <sys/io/io.h>



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
	fs_color=in_color*vec3(1.0,1.0,1-2*projection.z)+vec3(0.0,0.0,projection.z); \n\
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



int main(void){
	glsl_preprocessor_state_t preprocessor_state;
	glsl_preprocessor_state_init(&preprocessor_state);
	glsl_error_t error=glsl_preprocessor_add_file(vertex_shader_setup,0xffffffff,&preprocessor_state);
	if (error){
		goto _error;
	}
	error=glsl_preprocessor_add_file(vertex_shader,0,&preprocessor_state);
	if (error){
		goto _error;
	}
	// sys_io_print("%s\n",preprocessor_state.data);
	glsl_lexer_token_list_t token_list;
	error=glsl_lexer_extract_tokens(preprocessor_state.data,&token_list);
	glsl_preprocessor_state_delete(&preprocessor_state);
	if (error){
		goto _error;
	}
	// glsl_debug_print_token_list(&token_list);
	glsl_linker_program_t program;
	glsl_linker_program_init(&program);
	error=glsl_parser_parse_tokens(&token_list,&program,GLSL_SHADER_TYPE_VERTEX);
	glsl_lexer_delete_token_list(&token_list);
	if (error){
		goto _error;
	}
	glsl_debug_print_ast(program.shaders+GLSL_SHADER_TYPE_VERTEX);
	glsl_preprocessor_state_init(&preprocessor_state);
	error=glsl_preprocessor_add_file(fragment_shader_setup,0xffffffff,&preprocessor_state);
	if (error){
		goto _error;
	}
	error=glsl_preprocessor_add_file(fragment_shader,0,&preprocessor_state);
	if (error){
		goto _error;
	}
	// sys_io_print("%s\n",preprocessor_state.data);
	error=glsl_lexer_extract_tokens(preprocessor_state.data,&token_list);
	glsl_preprocessor_state_delete(&preprocessor_state);
	if (error){
		goto _error;
	}
	// glsl_debug_print_token_list(&token_list);
	error=glsl_parser_parse_tokens(&token_list,&program,GLSL_SHADER_TYPE_FRAGMENT);
	glsl_lexer_delete_token_list(&token_list);
	if (error){
		goto _error;
	}
	glsl_debug_print_ast(program.shaders+GLSL_SHADER_TYPE_FRAGMENT);
	glsl_linker_linked_program_t linked_program;
	error=glsl_linker_program_link(&program,NULL,&linked_program);
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
