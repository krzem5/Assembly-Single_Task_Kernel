#include <shell/command.h>
#include <shell/environment.h>
#include <sys/types.h>



static s64 _command(shell_environment_t* env,shell_command_context_t* ctx){
	env->close_current_session=1;
	if (env->exit_callback){
		return env->exit_callback(0);
	}
	return 0;
}



SYS_PUBLIC const shell_command_t shell_builtin_command_exit=_command;
