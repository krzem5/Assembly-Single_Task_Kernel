#ifndef _GLSL_DEBUG_H_
#define _GLSL_DEBUG_H_ 1
#include <glsl/ast.h>
#include <glsl/compiler.h>
#include <glsl/lexer.h>
#include <glsl/linker.h>



void glsl_debug_print_token_list(const glsl_lexer_token_list_t* token_list);



void glsl_debug_print_ast(const glsl_ast_t* ast);



void glsl_debug_print_compilation_output(const glsl_compilation_output_t* output);



void glsl_debug_print_linked_program(const glsl_linker_linked_program_t* linked_program);



#endif
