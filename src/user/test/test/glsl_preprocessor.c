#include <glsl/preprocessor.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/glsl_common.h>
#include <test/test.h>



static _Bool _check_line(const glsl_preprocessor_state_t* state,char marker,u32 expected_line,u32 expected_file){
	u32 offset=0;
	for (;state->data[offset]!=marker;offset++);
	u32 file;
	u32 line;
	return glsl_preprocessor_get_location(state,offset,&file,&line)&&file==expected_file&&line==expected_line;
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
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"",0));
	TEST_ASSERT(!sys_string_compare(state.data,"\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("version only");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"#version 330 core\n",0));
	TEST_ASSERT(!sys_string_compare(state.data,"\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("text only");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"\nvery long preprocessing text\n\twith inline #directives\n\t\r ",0));
	TEST_ASSERT(!sys_string_compare(state.data,"\nvery long preprocessing text\n\twith inline #directives\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("empty directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"line1\n#\n#",0));
	TEST_ASSERT(!sys_string_compare(state.data,"line1\n\n\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#invalid_directive",0),"Unknown preprocessor directive 'invalid_directive'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid version directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version",0),"Expected version, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version not-a-valid-number",0),"Expected version, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version 01736",0),"Unknown GLSL version '990'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version 330 invalid_profile",0),"Unsupported GLSL profile 'invalid_profile'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid define directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#define",0),"Expected macro name, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#define aaa\n#define aaa",0),"Preprocessor macro 'aaa' is already defined"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid undef directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#undef",0),"Expected macro name, got ???"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid line directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#line",0),"Expected line number, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#line not-a-valid-number",0),"Expected line number, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#line 0x2 not-a-valid-number",0),"Expected file number, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#line 0x2 2not-a-valid-number",0),"Decimal digit expected, got 'n'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid number");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version 1abc",0),"Decimal digit expected, got 'a'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version 078a",0),"Octal digit expected, got '8'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_preprocessor_add_file(&state,"#version 0xfg",0),"Hexadecimal digit expected, got 'g'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("macro deletion and expansion");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"#define macro1 aaa\n#define macro2 bbb\n#undef macro1\n#define macro1 new macro 1\ntest insert macro1 and macro2.",0));
	TEST_ASSERT(!sys_string_compare(state.data,"\n\n\n\ntest insert new macro 1 and bbb.\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_FUNC("glsl_preprocessor_get_location");
	TEST_GROUP("empty state");
	u32 tmp[2];
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_get_location(&state,0,tmp,tmp+1));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("out-of-bounds offset");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"text",0));
	TEST_ASSERT(!glsl_preprocessor_get_location(&state,state.length,tmp,tmp+1));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("correct args");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file(&state,"A\n#\nB\n#line 55\nC\n\nD\n#line 0 789\nE\n",123));
	TEST_ASSERT(_check_line(&state,'A',1,123));
	TEST_ASSERT(_check_line(&state,'B',3,123));
	TEST_ASSERT(_check_line(&state,'C',56,123));
	TEST_ASSERT(_check_line(&state,'D',58,123));
	TEST_ASSERT(_check_line(&state,'E',1,789));
	glsl_preprocessor_state_deinit(&state);
}
