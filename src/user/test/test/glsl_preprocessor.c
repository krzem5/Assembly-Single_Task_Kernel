#include <glsl/error.h>
#include <glsl/preprocessor.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



static _Bool _compare_and_cleanup_error(glsl_error_t error,const char* expected){
	TEST_ASSERT(error);
	if (!error){
		return 0;
	}
	_Bool out=!sys_string_compare(expected,error);
	glsl_error_delete(error);
	return out;
}



void test_glsl_preprocessor(void){
	TEST_MODULE("glsl_preprocessor");
	TEST_FUNC("glsl_preprocessor_state_init");
	TEST_GROUP("correct args");
	glsl_preprocessor_state_t state;
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!state.data);
	TEST_ASSERT(!state.length);
	glsl_preprocessor_state_deinit(&state);
	TEST_FUNC("glsl_preprocessor_add_file");
	TEST_GROUP("empty file");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file("",0,&state));
	TEST_ASSERT(!sys_string_compare(state.data,"\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("version only");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file("#version 330 core\n",0,&state));
	TEST_ASSERT(!sys_string_compare(state.data,"\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("text only");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file("\nvery long preprocessing text\n\twith inline #directives\n\t\r ",0,&state));
	TEST_ASSERT(!sys_string_compare(state.data,"\nvery long preprocessing text\n\twith inline #directives\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("empty directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file("line1\n#\n#",0,&state));
	TEST_ASSERT(!sys_string_compare(state.data,"line1\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#invalid_directive",0,&state),"Unknown preprocessor directive 'invalid_directive'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid version directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version",0,&state),"Expected version, got ???"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version not-a-valid-number",0,&state),"Expected version, got ???"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version 990",0,&state),"Unknown GLSL version '990'"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version 330 invalid_profile",0,&state),"Unsupported GLSL profile 'invalid_profile'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid define directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#define",0,&state),"Expected macro name, got ???"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#define aaa\n#define aaa",0,&state),"Preprocessor macro 'aaa' is already defined"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid undef directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#undef",0,&state),"Expected macro name, got ???"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid line directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#line",0,&state),"Expected line number, got ???"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#line not-a-valid-number",0,&state),"Expected line number, got ???"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#line 2 not-a-valid-number",0,&state),"Expected file number, got ???"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid number");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version 1abc",0,&state),"Decimal digit expected, got 'a'"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version 078a",0,&state),"Octal digit expected, got '8'"));
	TEST_ASSERT(_compare_and_cleanup_error(glsl_preprocessor_add_file("#version 0xfg",0,&state),"Hexadecimal digit expected, got 'g'"));
	glsl_preprocessor_state_deinit(&state);
}
