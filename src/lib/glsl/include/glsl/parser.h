#ifndef _GLSL_PARSER_H_
#define _GLSL_PARSER_H_ 1
#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>
#include <sys/types.h>



typedef struct _GLSL_PARSER_STATE{
	u32 shader_type;
	const glsl_lexer_token_t* tokens;
	u32 index;
	u32 length;
	glsl_ast_t* ast;
} glsl_parser_state_t;



glsl_error_t glsl_parser_parse_tokens(const glsl_lexer_token_list_t* token_list,glsl_linker_program_t* program,glsl_shader_type_t shader_type);



#endif
