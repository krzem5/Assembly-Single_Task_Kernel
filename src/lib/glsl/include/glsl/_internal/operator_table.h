#ifndef _GLSL__INTERNAL_OPERATOR_TABLE_H_
#define _GLSL__INTERNAL_OPERATOR_TABLE_H_ 1
#include <glsl/ast.h>
#include <glsl/error.h>
#include <sys/types.h>



typedef glsl_ast_node_t* (*glsl_unary_operator_t)(glsl_ast_node_t*,glsl_error_t*);



typedef glsl_ast_node_t* (*glsl_binary_operator_t)(glsl_ast_node_t*,glsl_ast_node_t*,glsl_error_t*);



typedef struct _GLSL_OPERATOR{
	const char* name;
	u8 unary_precedence;
	u8 binary_precedence;
	glsl_unary_operator_t unary;
	glsl_binary_operator_t binary;
} glsl_operator_t;



extern const glsl_operator_t _glsl_operator_table[];



#endif
