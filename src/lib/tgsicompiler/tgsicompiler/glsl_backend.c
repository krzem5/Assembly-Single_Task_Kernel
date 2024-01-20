#include <glsl/ast.h>
#include <glsl/backend.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/types.h>
#include <tgsicompiler/compiler.h>



static glsl_error_t _glsl_shader_link_callback(const glsl_ast_t* shader_ast,glsl_shader_type_t shader_type,glsl_linker_linked_program_shader_t* out){
	tgsi_compilation_output_t compilation_output;
	glsl_error_t error=tgsi_compile_shader(shader_ast,shader_type,&compilation_output);
	out->data=compilation_output.data;
	out->length=compilation_output.length;
	return error;
}



static const glsl_backend_descriptor_t _tgsi_glsl_backend_descriptor={
	"tgsi",
	_glsl_shader_link_callback
};



SYS_PUBLIC const glsl_backend_descriptor_t* _glsl_backend_query_descriptor(void){
	return &_tgsi_glsl_backend_descriptor;
}
