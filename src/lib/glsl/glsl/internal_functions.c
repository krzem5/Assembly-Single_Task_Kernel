#include <glsl/internal_functions.h>
#include <sys/types.h>



static const char*const _glsl_internal_function_type_to_string[GLSL_INTERNAL_FUNCTION_MAX_TYPE+1]={
	[GLSL_INTERNAL_FUNCTION_TYPE_NONE]="none",
	[GLSL_INTERNAL_FUNCTION_TYPE_TEXTURE_VEC4_SAMPLER_2D_VEC2]="vec4 texture(sampler2D,vec2)",
};



const char* glsl_internal_function_type_to_string(glsl_internal_function_type_t type){
	return _glsl_internal_function_type_to_string[type];
}
