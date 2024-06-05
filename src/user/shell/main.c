#include <input.h>
#include <shell/command.h>
#include <shell/environment.h>
#include <shell/interpreter.h>
#include <sys/acl/acl.h>
#include <sys/mp/process.h>
#include <sys/signal/signal.h>
#include <sys/types.h>



static const char*const _default_search_path[]={
	".",
	"/bin",
	NULL,
};

static s64 _exit_return_value=0;



static void _execute_callback(shell_environment_t* env,shell_command_context_t* ctx){
	sys_acl_set_permissions(sys_process_get_handle(),ctx->process,0,SYS_PROCESS_ACL_FLAG_SWITCH_USER);
}



static s64 _exit_callback(s64 return_value){
	_exit_return_value=return_value;
	return 0;
}



s64 main(u32 argc,const char*const* argv){
	sys_signal_set_mask(1<<SYS_SIGNAL_INTERRUPT,1);
	shell_environment_t* env=shell_environment_init(argc,argv,_execute_callback,_exit_callback,_default_search_path);
	shell_environment_add_builtin_commands_and_variables(env);
	while (!env->close_current_session){
		shell_interpreter_execute(env,input_get());
	}
	shell_environment_deinit(env);
	return _exit_return_value;
}
