#include <input.h>
#include <shell/interpreter.h>



static const char*const _default_search_path[]={
	".",
	"/bin",
	NULL,
};



void main(u32 argc,const char*const* argv){
	shell_environment_t* env=shell_environment_init(argc,argv,NULL,_default_search_path);
	shell_environment_add_builtin_commands_and_variables(env);
	while (1){
		shell_interpreter_execute(env,input_get());
	}
}
