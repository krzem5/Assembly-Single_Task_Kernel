#ifndef _GLSL_BACKEND_H_
#define _GLSL_BACKEND_H_ 1
#include <sys/types.h>



typedef struct _GLSL_BACKEND_DESCRIPTOR{
	const char* name;
} glsl_backend_descriptor_t;



typedef const glsl_backend_descriptor_t* (*glsl_backend_descriptor_query_func_t)(void);



#endif
