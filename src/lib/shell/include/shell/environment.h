#ifndef _SHELL_ENVIRONMENT_H_
#define _SHELL_ENVIRONMENT_H_ 1
#include <sys/fd/fd.h>
#include <sys/types.h>



typedef s64 (*shell_environment_shutdown_callback_t)(void);



typedef struct _SHELL_ENVIRONMENT{
	u32 argc;
	char** argv;
	shell_environment_shutdown_callback_t shutdown_callback;
	char** path;
	s64 last_return_value;
	sys_fd_t cwd_fd;
} shell_environment_t;



shell_environment_t* shell_environment_init(u32 argc,const char*const* argv,shell_environment_shutdown_callback_t shutdown_callback,const char*const* path);



void shell_environment_deinit(shell_environment_t* env);



#endif
