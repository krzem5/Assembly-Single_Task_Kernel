#include <glsl/version.h>
#include <sys/types.h>
#include <test/test.h>



void test_glsl_version(void){
	TEST_MODULE("glsl_version");
	TEST_FUNC("glsl_get_version");
	TEST_GROUP("correct args");
	TEST_ASSERT(glsl_get_version()==330);
}
