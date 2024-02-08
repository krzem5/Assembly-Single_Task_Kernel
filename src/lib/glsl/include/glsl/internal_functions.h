#ifndef _GLSL_INTERNAL_FUNCTIONS_H_
#define _GLSL_INTERNAL_FUNCTIONS_H_ 1
#include <sys/types.h>



#define GLSL_INTERNAL_FUNCTION_TYPE_NONE 0
#define GLSL_INTERNAL_FUNCTION_TYPE_TEXTURE_VEC4_SAMPLER_2D_VEC2 1

#define GLSL_INTERNAL_FUNCTION_MAX_TYPE GLSL_INTERNAL_FUNCTION_TYPE_TEXTURE_VEC4_SAMPLER_2D_VEC2



typedef u32 glsl_internal_function_type_t;



const char* glsl_internal_function_type_to_string(glsl_internal_function_type_t type);



#endif
