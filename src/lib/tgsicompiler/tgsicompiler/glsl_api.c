#include <glsl/ast.h>
#include <glsl/backend.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/types.h>



static glsl_error_t _glsl_shader_link_callback(glsl_ast_t* shader_ast,glsl_shader_type_t shader_type,glsl_linker_linked_program_shader_t* out){
	return GLSL_NO_ERROR;
}



static const glsl_backend_descriptor_t _tgsi_glsl_backend_descriptor={
	"tgsi",
	_glsl_shader_link_callback
};



SYS_PUBLIC const glsl_backend_descriptor_t* _glsl_backend_query_descriptor(void){
	return &_tgsi_glsl_backend_descriptor;
}
