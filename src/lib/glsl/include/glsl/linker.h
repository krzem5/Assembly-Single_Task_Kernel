#ifndef _GLSL_LINKER_H_
#define _GLSL_LINKER_H_ 1
#include <glsl/ast.h>
#include <glsl/error.h>
#include <sys/types.h>



#define GLSL_SHADER_TYPE_VERTEX 0
#define GLSL_SHADER_TYPE_FRAGMENT 1

#define GLSL_SHADER_MAX_TYPE GLSL_SHADER_TYPE_FRAGMENT



struct _GLSL_BACKEND_DESCRIPTOR;



typedef u32 glsl_shader_type_t;



typedef struct _GLSL_LINKER_LINKED_PROGRAM_SHADER{
	void* data;
	u32 length;
} glsl_linker_linked_program_shader_t;



typedef struct _GLSL_LINKER_LINKED_PROGRAM_UNIFORM{
	char* name;
	u32 slot;
	u32 size;
	glsl_ast_type_t* _type;
} glsl_linker_linked_program_uniform_t;



typedef struct _GLSL_LINKER_PROGRAM{
	u32 shader_bitmap;
	glsl_ast_t shaders[GLSL_SHADER_MAX_TYPE+1];
} glsl_linker_program_t;



typedef struct _GLSL_LINKER_LINKED_PROGRAM{
	u32 shader_bitmap;
	u32 uniform_count;
	glsl_linker_linked_program_shader_t shaders[GLSL_SHADER_MAX_TYPE+1];
	glsl_linker_linked_program_uniform_t* uniforms;
} glsl_linker_linked_program_t;



void glsl_linker_program_init(glsl_linker_program_t* program);



void glsl_linker_program_delete(glsl_linker_program_t* program);



void glsl_linker_linked_program_delete(glsl_linker_linked_program_t* linked_program);



glsl_error_t glsl_linker_program_link(glsl_linker_program_t* program,const struct _GLSL_BACKEND_DESCRIPTOR* backend,glsl_linker_linked_program_t* out);



#endif
