#include <glsl/error.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



_Bool test_glsl_check_and_cleanup_error(glsl_error_t error,const char* expected){
	_Bool out=0;
	if (error){
		out=!sys_string_compare(expected,error);
		glsl_error_delete(error);
	}
	return out;
}
