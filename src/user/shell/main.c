#include <input.h>
#include <shell/interpreter.h>
#include <sys/mp/process.h>
#include <sys/system/system.h>
#include <sys/types.h>



static const char*const _default_search_path[]={
	".",
	"/bin",
	NULL,
};

static s64 _exit_return_value=0;



static s64 _exit_callback(s64 return_value){
	_exit_return_value=return_value;
	return 0;
}



s64 main(u32 argc,const char*const* argv){
	shell_environment_t* env=shell_environment_init(argc,argv,_exit_callback,_default_search_path);
	shell_environment_add_builtin_commands_and_variables(env);
	while (!env->close_current_session){
		shell_interpreter_execute(env,input_get());
	}
	shell_environment_deinit(env);
	if (!(sys_process_get_parent(0)>>16)){
		sys_system_shutdown(0);
	}
	return _exit_return_value;
}
