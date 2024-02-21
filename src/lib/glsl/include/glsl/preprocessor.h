#ifndef _GLSL_PREPROCESSOR_H_
#define _GLSL_PREPROCESSOR_H_ 1
#include <glsl/error.h>
#include <sys/types.h>



typedef struct _GLSL_PREPROCESSOR_LINE{
	u32 offset;
	u32 file;
	u32 line;
} glsl_preprocessor_line_t;



typedef struct _GLSL_PREPROCESSOR_MACRO{
	u32 identifier_length;
	char* identifier;
	char* value;
} glsl_preprocessor_macro_t;



typedef struct _GLSL_PREPROCESSOR_STATE{
	char* data;
	u32 length;
	u32 _capacity;
	glsl_preprocessor_line_t* lines;
	glsl_preprocessor_macro_t* macros;
	u32 line_count;
	u32 macro_count;
} glsl_preprocessor_state_t;



void glsl_preprocessor_state_init(glsl_preprocessor_state_t* state);



void glsl_preprocessor_state_deinit(glsl_preprocessor_state_t* state);



glsl_error_t glsl_preprocessor_add_file(const char* src,u32 index,glsl_preprocessor_state_t* state);



#endif
