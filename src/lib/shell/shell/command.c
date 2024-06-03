#include <shell/command.h>
#include <shell/environment.h>
#include <sys/acl/acl.h>
#include <sys/container/container.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/string/string.h>
#include <sys/types.h>



SYS_PUBLIC shell_command_context_t* shell_command_context_create(void){
	shell_command_context_t* out=sys_heap_alloc(NULL,sizeof(shell_command_context_t));
	out->flags=0;
	out->argc=0;
	out->argv=NULL;
	out->stdin=sys_io_input_fd;
	out->stdout=sys_io_output_fd;
	out->stderr=sys_io_error_fd;
	out->process=0;
	out->return_value=0;
	return out;
}



SYS_PUBLIC void shell_command_context_delete_args(shell_command_context_t* ctx){
	for (u32 i=0;i<ctx->argc;i++){
		sys_heap_dealloc(NULL,ctx->argv[i]);
	}
	sys_heap_dealloc(NULL,ctx->argv);
	ctx->argc=0;
	ctx->argv=NULL;
}



SYS_PUBLIC void shell_command_context_dispatch(shell_command_context_t* ctx,shell_environment_t* env,bool wait_for_result){
	if (!ctx->argc){
		goto _cleanup;
	}
	for (u32 i=0;i<env->command_count;i++){
		if (!sys_string_compare((env->commands+i)->name,ctx->argv[0])){
			ctx->return_value=(env->commands+i)->command(env,ctx);
			goto _cleanup;
		}
	}
	for (u32 i=0;env->path[i];i++){
		sys_fd_t parent_fd=sys_fd_open(env->cwd_fd,env->path[i],0);
		if (SYS_IS_ERROR(parent_fd)){
			continue;
		}
		sys_fd_t fd=sys_fd_open(parent_fd,ctx->argv[0],0);
		sys_fd_close(parent_fd);
		sys_fd_stat_t stat;
		if (SYS_IS_ERROR(fd)||SYS_IS_ERROR(sys_fd_stat(fd,&stat))||stat.type!=SYS_FD_STAT_TYPE_FILE){
			continue;
		}
		char path[4096];
		sys_fd_path(fd,path,4096);
		sys_fd_close(fd);
		ctx->process=sys_process_start(path,ctx->argc,(const char*const*)(ctx->argv),NULL,SYS_PROCESS_START_FLAG_PAUSE_THREAD,ctx->stdin,ctx->stdout,ctx->stderr);
		if (SYS_IS_ERROR(ctx->process)){
			sys_io_print("error: unable to execute file '%s': %ld\n",path,ctx->process);
			ctx->process=0;
			goto _cleanup;
		}
		sys_acl_set_permissions(ctx->stdin,ctx->process,0,SYS_FD_ACL_FLAG_DUP);
		sys_acl_set_permissions(ctx->stdout,ctx->process,0,SYS_FD_ACL_FLAG_DUP);
		sys_acl_set_permissions(ctx->stderr,ctx->process,0,SYS_FD_ACL_FLAG_DUP);
		if (env->execute_callback){
			env->execute_callback(env,ctx);
		}
		if (!wait_for_result){
			sys_thread_start(sys_process_get_main_thread(ctx->process));
			shell_command_context_delete_args(ctx);
			return;
		}
		sys_container_t container=sys_container_create();
		sys_container_add(container,&(ctx->process),1);
		sys_thread_start(sys_process_get_main_thread(ctx->process));
		shell_command_context_delete_args(ctx);
		sys_thread_await_event(sys_process_get_termination_event(ctx->process));
		ctx->return_value=(s64)(u64)sys_process_get_return_value(ctx->process);
		sys_container_delete(container);
		return;
	}
	sys_io_print("error: unable to find command '%s'\n",ctx->argv[0]);
	ctx->return_value=-1;
_cleanup:
	shell_command_context_delete_args(ctx);
}



SYS_PUBLIC void shell_command_context_delete(shell_command_context_t* ctx){
	shell_command_context_delete_args(ctx);
	if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDIN){
		sys_fd_close(ctx->stdin);
	}
	if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
		sys_fd_close(ctx->stdout);
	}
	if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDERR){
		sys_fd_close(ctx->stderr);
	}
	sys_heap_dealloc(NULL,ctx);
}
