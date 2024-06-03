#include <shell/command.h>
#include <shell/environment.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/mp/process.h>
#include <sys/types.h>



static s64 _command(shell_environment_t* env,shell_command_context_t* ctx){
	if (ctx->argc<2){
		sys_io_print("chroot: not enough arguments\n");
		return 1;
	}
	else if (ctx->argc>2){
		sys_io_print("chroot: too many arguments\n");
		return 1;
	}
	sys_fd_t fd=sys_fd_open(env->cwd_fd,ctx->argv[1],0);
	if (SYS_IS_ERROR(fd)){
		sys_io_print("chroot: unable to change current directory to '%s'\n",ctx->argv[1]);
		return 1;
	}
	if (SYS_IS_ERROR(sys_process_set_root(0,fd))){
		sys_fd_close(fd);
		sys_io_print("chroot: unable to change current directory to '%s'\n",ctx->argv[1]);
		return 1;
	}
	sys_fd_close(fd);
	return 0;
}



SYS_PUBLIC const shell_command_t shell_builtin_command_chroot=_command;
