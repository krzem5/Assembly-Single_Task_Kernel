#include <shell/environment.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/string/string.h>
#include <sys/types.h>



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
	sys_heap_dealloc(NULL,env);
}
