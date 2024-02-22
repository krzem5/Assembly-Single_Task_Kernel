#include <glsl/error.h>
#include <glsl/preprocessor.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



static _Bool _check_and_cleanup_error(glsl_error_t error,const char* expected){
	TEST_ASSERT(error);
	_Bool out=0;
	if (error){
		out=!sys_string_compare(expected,error);
		glsl_error_delete(error);
	}
	return out;
}



static _Bool _check_line(const glsl_preprocessor_state_t* state,char marker,u32 expected_line,u32 expected_file){
	if (!state->line_count){
		return 0;
	}
	u32 offset=0;
	for (;state->data[offset]!=marker;offset++);
	u32 i=0;
	for (;i+1<state->line_count&&(state->lines+i+1)->offset<=offset;i++);
	u32 line=(state->lines+i)->line;
	for (;offset>(state->lines+i)->offset;offset--){
		if (state->data[offset]=='\n'){
			line++;
		}
	}
	return (state->lines+i)->file==expected_file&&line==expected_line;
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
	TEST_ASSERT(!sys_string_compare(state.data,"line1\n\n\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#invalid_directive",0,&state),"Unknown preprocessor directive 'invalid_directive'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid version directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version",0,&state),"Expected version, got ???"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version not-a-valid-number",0,&state),"Expected version, got ???"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version 01736",0,&state),"Unknown GLSL version '990'"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version 330 invalid_profile",0,&state),"Unsupported GLSL profile 'invalid_profile'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid define directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#define",0,&state),"Expected macro name, got ???"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#define aaa\n#define aaa",0,&state),"Preprocessor macro 'aaa' is already defined"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid undef directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#undef",0,&state),"Expected macro name, got ???"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid line directive");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#line",0,&state),"Expected line number, got ???"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#line not-a-valid-number",0,&state),"Expected line number, got ???"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#line 0x2 not-a-valid-number",0,&state),"Expected file number, got ???"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#line 0x2 2not-a-valid-number",0,&state),"Decimal digit expected, got 'n'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("invalid number");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version 1abc",0,&state),"Decimal digit expected, got 'a'"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version 078a",0,&state),"Octal digit expected, got '8'"));
	TEST_ASSERT(_check_and_cleanup_error(glsl_preprocessor_add_file("#version 0xfg",0,&state),"Hexadecimal digit expected, got 'g'"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("macro deletion and expansion");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file("#define macro1 aaa\n#define macro2 bbb\n#undef macro1\n#define macro1 new macro 1\ntest insert macro1 and macro2.",0,&state));
	TEST_ASSERT(!sys_string_compare(state.data,"\n\n\n\ntest insert new macro 1 and bbb.\n"));
	glsl_preprocessor_state_deinit(&state);
	TEST_GROUP("line numbering");
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!glsl_preprocessor_add_file("A\n#\nB\n#line 55\nC\n\nD\n#line 0 789\nE\n",123,&state));
	TEST_ASSERT(_check_line(&state,'A',1,123));
	TEST_ASSERT(_check_line(&state,'B',3,123));
	TEST_ASSERT(_check_line(&state,'C',56,123));
	TEST_ASSERT(_check_line(&state,'D',58,123));
	TEST_ASSERT(_check_line(&state,'E',1,789));
	glsl_preprocessor_state_deinit(&state);
}
