#include <glsl/error.h>
#include <glsl/lexer.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



void test_glsl_lexer(void){
	TEST_MODULE("glsl_lexer");
	TEST_FUNC("glsl_lexer_extract_tokens");
	TEST_GROUP("empty input");
	glsl_lexer_token_list_t token_list;
	TEST_ASSERT(!glsl_lexer_extract_tokens("",&token_list));
	TEST_ASSERT(!token_list.length);
	glsl_lexer_delete_token_list(&token_list);
}
