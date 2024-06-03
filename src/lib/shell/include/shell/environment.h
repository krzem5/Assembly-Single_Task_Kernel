#ifndef _SHELL_ENVIRONMENT_H_
#define _SHELL_ENVIRONMENT_H_ 1
#include <shell/command.h>
#include <sys/fd/fd.h>
#include <sys/types.h>



struct _SHELL_ENVIRONMENT;



typedef char* (*shell_environment_dynamic_variable_callback_t)(struct _SHELL_ENVIRONMENT*,void*);



typedef s64 (*shell_environment_exit_callback_t)(s64);



typedef struct _SHELL_ENVIRONMENT_COMMAND{
	char* name;
	shell_command_t command;
} shell_environment_command_t;



typedef struct _SHELL_ENVIRONMENT_VARIABLE{
	char* name;
	u32 name_length;
	bool is_dynamic;
	union{
		struct{
			char* value;
			u32 value_length;
		} static_;
		struct{
			shell_environment_dynamic_variable_callback_t callback;
			void* ctx;
		} dynamic;
	};
} shell_environment_variable_t;



typedef struct _SHELL_ENVIRONMENT{
	u32 argc;
	char** argv;
	shell_environment_exit_callback_t exit_callback;
	char** path;
	u32 command_count;
	shell_environment_command_t* commands;
	u32 variable_count;
	shell_environment_variable_t* variables;
	s64 last_return_value;
	sys_fd_t cwd_fd;
	bool close_current_session;
} shell_environment_t;



shell_environment_t* shell_environment_init(u32 argc,const char*const* argv,shell_environment_exit_callback_t exit_callback,const char*const* path);



void shell_environment_deinit(shell_environment_t* env);



void shell_environment_add_command(shell_environment_t* env,const char* name,shell_command_t command);



void shell_environment_add_static_variable(shell_environment_t* env,const char* name,const char* value);



void shell_environment_add_dynamic_variable(shell_environment_t* env,const char* name,shell_environment_dynamic_variable_callback_t callback,void* ctx);



char* shell_environment_get_variable(shell_environment_t* env,const char* name,u32 name_length);



void shell_environment_add_builtin_commands_and_variables(shell_environment_t* env);



#endif
