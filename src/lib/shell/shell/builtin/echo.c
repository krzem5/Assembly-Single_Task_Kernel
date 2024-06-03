#include <shell/command.h>
#include <shell/environment.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>



static s64 _command(shell_environment_t* env,shell_command_context_t* ctx){
	for (u32 i=1;i<ctx->argc;i++){
		if (SYS_IS_ERROR(sys_fd_write(sys_io_output_fd,ctx->argv[i],sys_string_length(ctx->argv[i]),0))){
			return 1;
		}
	}
	sys_io_print("\n");
	return 0;
}



SYS_PUBLIC const shell_command_t shell_builtin_command_echo=_command;
