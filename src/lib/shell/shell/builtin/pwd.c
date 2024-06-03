#include <shell/command.h>
#include <shell/environment.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/types.h>



static s64 _command(shell_environment_t* env,shell_command_context_t* ctx){
	char buffer[4096];
	sys_fd_path(env->cwd_fd,buffer,sizeof(buffer));
	sys_io_print("%s\n",buffer);
	return 0;
}



SYS_PUBLIC const shell_command_t shell_builtin_command_pwd=_command;
