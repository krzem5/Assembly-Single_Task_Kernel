#ifndef _SHELL_COMMAND_H_
#define _SHELL_COMMAND_H_ 1
#include <sys/fd/fd.h>
#include <sys/mp/process.h>
#include <sys/types.h>



#define SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDIN 1
#define SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT 2
#define SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDERR 4



typedef struct _SHELL_COMMAND_CONTEXT{
	u32 flags;
	u32 argc;
	char** argv;
	sys_fd_t stdin;
	sys_fd_t stdout;
	sys_fd_t stderr;
	sys_process_t process;
	s64 return_value;
} shell_command_context_t;



struct _SHELL_ENVIRONMENT;



typedef s64 (*shell_command_t)(struct _SHELL_ENVIRONMENT*,shell_command_context_t*);



shell_command_context_t* shell_command_context_create(void);



void shell_command_context_delete_args(shell_command_context_t* ctx);



void shell_command_context_dispatch(shell_command_context_t* ctx,struct _SHELL_ENVIRONMENT* env,bool wait_for_result);



void shell_command_context_delete(shell_command_context_t* ctx);



#endif
