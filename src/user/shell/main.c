#include <shell/command.h>
#include <shell/environment.h>
#include <shell/interpreter.h>
#include <sys/acl/acl.h>
#include <sys/fd/fd.h>
#include <sys/id/user.h>
#include <sys/io/io.h>
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
		char cwd_buffer[4096]="???";
		sys_fd_path(env->cwd_fd,cwd_buffer,sizeof(cwd_buffer));
		char uid_name_buffer[256]="???";
		sys_uid_get_name(sys_uid_get(),uid_name_buffer,256);
		sys_io_print("\x1b[G\x1b[2K\x1b[1m\x1b[32m%s\x1b[0m:\x1b[1m\x1b[34m%s\x1b[0m$ ",uid_name_buffer,cwd_buffer);
		char line_buffer[4096];
		sys_error_t count=sys_fd_read(sys_io_input_fd,line_buffer,sizeof(line_buffer)-1,0);
		if (!count||SYS_IS_ERROR(count)){
			_exit_return_value=12345;
			break;
		}
		line_buffer[count]=0;
		shell_interpreter_execute(env,line_buffer);
	}
	shell_environment_deinit(env);
	return _exit_return_value;
}
