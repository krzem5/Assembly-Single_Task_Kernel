#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/lexer.h>
#include <glsl/parser.h>
#include <glsl/shader.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/glsl_common.h>
#include <test/test.h>



static glsl_error_t _execute_parser(const char* src,glsl_shader_type_t shader_type,glsl_ast_t* out){
	glsl_lexer_token_list_t token_list;
	TEST_ASSERT(!glsl_lexer_extract_tokens(src,&token_list));
	glsl_error_t error=glsl_parser_parse_tokens(&token_list,shader_type,out);
	glsl_lexer_delete_token_list(&token_list);
	return error;
}



void test_glsl_parser(void){
	TEST_MODULE("glsl_parser");
	TEST_FUNC("glsl_parser_parse_tokens");
	TEST_GROUP("empty input");
	glsl_ast_t ast;
	TEST_ASSERT(!_execute_parser("",GLSL_SHADER_TYPE_ANY,&ast));
	glsl_ast_delete(&ast);
	TEST_GROUP("precision");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision",GLSL_SHADER_TYPE_ANY,&ast),"Expected precision qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision 10.5",GLSL_SHADER_TYPE_ANY,&ast),"Expected precision qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision lowp",GLSL_SHADER_TYPE_ANY,&ast),"Expected type, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision mediump",GLSL_SHADER_TYPE_ANY,&ast),"Expected type, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision highp",GLSL_SHADER_TYPE_ANY,&ast),"Expected type, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision lowp int",GLSL_SHADER_TYPE_ANY,&ast),"Expected semicolon, got ???"));
	TEST_ASSERT(!_execute_parser("precision mediump float;",GLSL_SHADER_TYPE_ANY,&ast));
	glsl_ast_delete(&ast);
}
