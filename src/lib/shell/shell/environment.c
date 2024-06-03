#include <shell/builtin/cd.h>
#include <shell/builtin/chroot.h>
#include <shell/builtin/echo.h>
#include <shell/builtin/exit.h>
#include <shell/builtin/pwd.h>
#include <shell/environment.h>
#include <sys/fd/fd.h>
#include <sys/format/format.h>
#include <sys/heap/heap.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



static char* _last_return_code_callback(shell_environment_t* env,void* ctx){
	char buffer[32];
	sys_format_string(buffer,sizeof(buffer),"%ld",env->last_return_value);
	return sys_string_duplicate(buffer);
}



static char* _argv_callback(shell_environment_t* env,void* ctx){
	return sys_string_duplicate(env->argv[(u64)ctx]);
}



SYS_PUBLIC shell_environment_t* shell_environment_init(u32 argc,const char*const* argv,shell_environment_shutdown_callback_t shutdown_callback,const char*const* path){
	shell_environment_t* out=sys_heap_alloc(NULL,sizeof(shell_environment_t));
	out->argc=argc;
	out->argv=sys_heap_alloc(NULL,argc*sizeof(char*));
	for (u32 i=0;i<argc;i++){
		out->argv[i]=sys_string_duplicate(argv[i]);
	}
	out->shutdown_callback=shutdown_callback;
	u32 path_length=0;
	for (;path[path_length];path_length++);
	out->path=sys_heap_alloc(NULL,(path_length+1)*sizeof(char*));
	for (u32 i=0;i<path_length;i++){
		out->path[i]=sys_string_duplicate(path[i]);
	}
	out->path[path_length]=NULL;
	out->command_count=0;
	out->commands=NULL;
	out->variable_count=0;
	out->variables=NULL;
	out->last_return_value=0;
	out->cwd_fd=sys_fd_dup(SYS_FD_DUP_CWD,0);
	return out;
}



SYS_PUBLIC void shell_environment_deinit(shell_environment_t* env){
	for (u32 i=0;i<env->argc;i++){
		sys_heap_dealloc(NULL,env->argv[i]);
	}
	sys_heap_dealloc(NULL,env->argv);
	for (u32 i=0;env->path[i];i++){
		sys_heap_dealloc(NULL,env->path[i]);
	}
	sys_heap_dealloc(NULL,env->path);
	for (u32 i=0;i<env->command_count;i++){
		sys_heap_dealloc(NULL,(env->commands+i)->name);
	}
	sys_heap_dealloc(NULL,env->commands);
	for (u32 i=0;i<env->variable_count;i++){
		sys_heap_dealloc(NULL,(env->variables+i)->name);
		if (!(env->variables+i)->is_dynamic){
			sys_heap_dealloc(NULL,(env->variables+i)->static_.value);
		}
	}
	sys_heap_dealloc(NULL,env->variables);
	sys_heap_dealloc(NULL,env);
}



SYS_PUBLIC void shell_environment_add_command(shell_environment_t* env,const char* name,shell_command_t command){
	env->command_count++;
	env->commands=sys_heap_realloc(NULL,env->commands,env->command_count*sizeof(shell_environment_command_t));
	(env->commands+env->command_count-1)->name=sys_string_duplicate(name);
	(env->commands+env->command_count-1)->command=command;
}



SYS_PUBLIC void shell_environment_add_static_variable(shell_environment_t* env,const char* name,const char* value){
	env->variable_count++;
	env->variables=sys_heap_realloc(NULL,env->variables,env->variable_count*sizeof(shell_environment_variable_t));
	shell_environment_variable_t* var=env->variables+env->variable_count-1;
	var->name_length=sys_string_length(name);
	var->name=sys_heap_alloc(NULL,var->name_length+1);
	sys_memory_copy(name,var->name,var->name_length+1);
	var->is_dynamic=0;
	var->static_.value_length=sys_string_length(value);
	var->static_.value=sys_heap_alloc(NULL,var->static_.value_length+1);
	sys_memory_copy(value,var->static_.value,var->static_.value_length+1);
}



SYS_PUBLIC void shell_environment_add_dynamic_variable(shell_environment_t* env,const char* name,shell_environment_dynamic_variable_callback_t callback,void* ctx){
	env->variable_count++;
	env->variables=sys_heap_realloc(NULL,env->variables,env->variable_count*sizeof(shell_environment_variable_t));
	shell_environment_variable_t* var=env->variables+env->variable_count-1;
	var->name_length=sys_string_length(name);
	var->name=sys_heap_alloc(NULL,var->name_length+1);
	sys_memory_copy(name,var->name,var->name_length+1);
	var->is_dynamic=1;
	var->dynamic.callback=callback;
	var->dynamic.ctx=ctx;
}



SYS_PUBLIC char* shell_environment_get_variable(shell_environment_t* env,const char* name,u32 name_length){
	if (!name_length){
		name_length=sys_string_length(name);
	}
	for (u32 i=0;i<env->variable_count;i++){
		if ((env->variables+i)->name_length!=name_length||sys_memory_compare((env->variables+i)->name,name,name_length)){
			continue;
		}
		if ((env->variables+i)->is_dynamic){
			return (env->variables+i)->dynamic.callback(env,(env->variables+i)->dynamic.ctx);
		}
		char* out=sys_heap_alloc(NULL,(env->variables+i)->static_.value_length+1);
		sys_memory_copy((env->variables+i)->static_.value,out,(env->variables+i)->static_.value_length+1);
		return out;
	}
	return NULL;
}



SYS_PUBLIC void shell_environment_add_builtin_commands_and_variables(shell_environment_t* env){
	shell_environment_add_command(env,"cd",shell_builtin_command_cd);
	shell_environment_add_command(env,"chroot",shell_builtin_command_chroot);
	shell_environment_add_command(env,"echo",shell_builtin_command_echo);
	shell_environment_add_command(env,"exit",shell_builtin_command_exit);
	shell_environment_add_command(env,"pwd",shell_builtin_command_pwd);
	shell_environment_add_dynamic_variable(env,"?",_last_return_code_callback,NULL);
	for (u32 i=0;i<env->argc;i++){
		char buffer[32];
		sys_format_string(buffer,sizeof(buffer),"%u",i);
		shell_environment_add_dynamic_variable(env,buffer,_argv_callback,(void*)(u64)i);
	}
}
