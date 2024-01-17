#ifndef _GLSL_ERROR_H_
#define _GLSL_ERROR_H_ 1



#define GLSL_NO_ERROR NULL



typedef char* glsl_error_t;



void glsl_error_delete(glsl_error_t error);



#endif
