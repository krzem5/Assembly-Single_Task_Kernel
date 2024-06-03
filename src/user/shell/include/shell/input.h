#ifndef _SHELL_INPUT_H_
#define _SHELL_INPUT_H_ 1
#include <sys/types.h>



#define INPUT_REWIND_BUFFER_SIZE 64
#define INPUT_BUFFER_SIZE 256



const char* input_get(void);



#endif
