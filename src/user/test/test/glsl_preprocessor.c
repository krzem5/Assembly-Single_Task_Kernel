#include <glsl/preprocessor.h>
#include <sys/types.h>
#include <test/test.h>



void test_glsl_preprocessor(void){
	TEST_MODULE("glsl_preprocessor");
	TEST_FUNC("glsl_preprocessor_state_init");
	TEST_GROUP("correct args");
	glsl_preprocessor_state_t state;
	glsl_preprocessor_state_init(&state);
	TEST_ASSERT(!state.data);
	TEST_ASSERT(!state.length);
	glsl_preprocessor_state_deinit(&state);
}
