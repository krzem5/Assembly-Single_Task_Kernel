#ifndef _TGSICOMPILER_COMPILER_H_
#define _TGSICOMPILER_COMPILER_H_ 1
#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/types.h>



typedef struct _TGSI_COMPILATION_OUTPUT{
	char* data;
	u32 length;
	u32 _capacity;
} tgsi_compilation_output_t;



glsl_error_t tgsi_compile_shader(const glsl_compilation_output_t* output,glsl_shader_type_t shader_type,tgsi_compilation_output_t* out);



#endif
