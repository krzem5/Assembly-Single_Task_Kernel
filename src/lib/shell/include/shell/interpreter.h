#ifndef _SHELL_INTERPRETER_H_
#define _SHELL_INTERPRETER_H_ 1
#include <shell/environment.h>
#include <sys/types.h>



void shell_interpreter_execute(shell_environment_t* env,const char* command);



#endif
