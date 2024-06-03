#include <shell/command.h>
#include <shell/environment.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/system/system.h>
#include <sys/types.h>



static s64 _command(shell_environment_t* env,shell_command_context_t* ctx){
	if (sys_process_get_parent(0)>>16){
		sys_thread_stop(0,NULL);
	}
	sys_system_shutdown(0);
	return 0;
}



SYS_PUBLIC const shell_command_t shell_builtin_command_exit=_command;
