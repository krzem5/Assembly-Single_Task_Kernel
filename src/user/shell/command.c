#include <command.h>
#include <cwd.h>
#include <input.h>
#include <string.h>
#include <sys/acl/acl.h>
#include <sys/container/container.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/string/string.h>
#include <sys/system/system.h>
#include <sys/types.h>



#define COMMAND_PARSER_IS_WHITESPACE(c) ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r')
#define COMMAND_PARSER_IS_VALID_CHARACTER(c) ((c)&&!COMMAND_PARSER_IS_WHITESPACE(c)&&(c)!='>'&&(c)!='<'&&(c)!='|'&&(c)!='&'&&(c)!=';')

#define COMMAND_PARSER_STATE_ARGUMENTS 0
#define COMMAND_PARSER_STATE_OPERATOR 1
#define COMMAND_PARSER_STATE_READ 2
#define COMMAND_PARSER_STATE_WRITE 3
#define COMMAND_PARSER_STATE_APPEND 4

#define COMMAND_CONTEXT_FLAG_CLOSE_STDIN 1
#define COMMAND_CONTEXT_FLAG_CLOSE_STDOUT 2
#define COMMAND_CONTEXT_FLAG_CLOSE_STDERR 4

#define COMMAND_EXECUTE_MODIFIER_ALWAYS 0
#define COMMAND_EXECUTE_MODIFIER_AND 1
#define COMMAND_EXECUTE_MODIFIER_OR 2



typedef struct _COMMAND_CONTEXT{
	u32 flags;
	u32 argc;
	char** argv;
	sys_fd_t stdin;
	sys_fd_t stdout;
	sys_fd_t stderr;
	sys_process_t process;
	s64 return_value;
} command_context_t;



static const char* _search_path[]={
	".",
	"/bin",
	NULL
};



static int _handle_cd(command_context_t* ctx){
	if (ctx->argc<2){
		sys_io_print("cd: not enough arguments\n");
		return 1;
	}
	else if (ctx->argc>2){
		sys_io_print("cd: too many arguments\n");
		return 1;
	}
	if (!cwd_change(ctx->argv[1])){
		sys_io_print("cd: unable to change current directory to '%s'\n",ctx->argv[1]);
		return 1;
	}
	return 0;
}



static int _handle_chroot(command_context_t* ctx){
	if (ctx->argc<2){
		sys_io_print("chroot: not enough arguments\n");
		return 1;
	}
	else if (ctx->argc>2){
		sys_io_print("chroot: too many arguments\n");
		return 1;
	}
	if (!cwd_change_root(ctx->argv[1])){
		sys_io_print("chroot: unable to change current directory to '%s'\n",ctx->argv[1]);
		return 1;
	}
	return 0;
}



static int _handle_echo(command_context_t* ctx){
	for (u32 i=1;i<ctx->argc;i++){
		if (SYS_IS_ERROR(sys_fd_write(sys_io_output_fd,ctx->argv[i],sys_string_length(ctx->argv[i]),0))){
			return 1;
		}
	}
	sys_io_print("\n");
	return 0;
}



static int _handle_exit(command_context_t* ctx){
	if (sys_process_get_parent(0)>>16){
		sys_thread_stop(0,NULL);
	}
	sys_system_shutdown(0);
	return 0;
}



static int _handle_pwd(command_context_t* ctx){
	sys_io_print("%s\n",cwd);
	return 0;
}



static const void* _internal_commands[]={
	"cd",_handle_cd,
	"chroot",_handle_chroot,
	"echo",_handle_echo,
	"exit",_handle_exit,
	"pwd",_handle_pwd,
	NULL,
};



static command_context_t* _create_command_context(void){
	command_context_t* out=sys_heap_alloc(NULL,sizeof(command_context_t));
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



static void _cleanup_command_context(command_context_t* ctx){
	for (u32 i=0;i<ctx->argc;i++){
		sys_heap_dealloc(NULL,ctx->argv[i]);
	}
	sys_heap_dealloc(NULL,ctx->argv);
	ctx->argc=0;
	ctx->argv=NULL;
}



static void _dispatch_command_context(command_context_t* ctx,bool wait_for_result){
	if (!ctx->argc){
		goto _cleanup;
	}
	for (u32 i=0;_internal_commands[i];i+=2){
		if (!sys_string_compare(_internal_commands[i],ctx->argv[0])){
			ctx->return_value=((int (*)(command_context_t*))(_internal_commands[i+1]))(ctx);
			goto _cleanup;
		}
	}
	for (u32 i=0;_search_path[i];i++){
		sys_fd_t parent_fd=sys_fd_open(cwd_fd,_search_path[i],0);
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
		sys_acl_set_permissions(sys_process_get_handle(),ctx->process,0,SYS_PROCESS_ACL_FLAG_SWITCH_USER);
		sys_acl_set_permissions(ctx->stdin,ctx->process,0,SYS_FD_ACL_FLAG_DUP);
		sys_acl_set_permissions(ctx->stdout,ctx->process,0,SYS_FD_ACL_FLAG_DUP);
		sys_acl_set_permissions(ctx->stderr,ctx->process,0,SYS_FD_ACL_FLAG_DUP);
		if (!wait_for_result){
			sys_thread_start(sys_process_get_main_thread(ctx->process));
			_cleanup_command_context(ctx);
			return;
		}
		sys_container_t container=sys_container_create();
		sys_container_add(container,&(ctx->process),1);
		sys_thread_start(sys_process_get_main_thread(ctx->process));
		_cleanup_command_context(ctx);
		sys_thread_await_event(sys_process_get_termination_event(ctx->process));
		ctx->return_value=(s64)(u64)sys_process_get_return_value(ctx->process);
		sys_container_delete(container);
		return;
	}
	sys_io_print("error: unable to find command '%s'\n",ctx->argv[0]);
	ctx->return_value=-1;
_cleanup:
	_cleanup_command_context(ctx);
}



static void _delete_command_context(command_context_t* ctx){
	_cleanup_command_context(ctx);
	if (ctx->flags&COMMAND_CONTEXT_FLAG_CLOSE_STDIN){
		ctx->flags&=~COMMAND_CONTEXT_FLAG_CLOSE_STDIN;
		sys_fd_close(ctx->stdin);
		ctx->stdin=0;
	}
	if (ctx->flags&COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
		ctx->flags&=~COMMAND_CONTEXT_FLAG_CLOSE_STDOUT;
		sys_fd_close(ctx->stdout);
		ctx->stdout=0;
	}
	if (ctx->flags&COMMAND_CONTEXT_FLAG_CLOSE_STDERR){
		ctx->flags&=~COMMAND_CONTEXT_FLAG_CLOSE_STDERR;
		sys_fd_close(ctx->stderr);
		ctx->stderr=0;
	}
	sys_heap_dealloc(NULL,ctx);
}



static bool _check_execute(u32 execute_modifier,s64 last_command_return_value){
	if (execute_modifier==COMMAND_EXECUTE_MODIFIER_ALWAYS){
		return 1;
	}
	if (execute_modifier==COMMAND_EXECUTE_MODIFIER_AND){
		return !last_command_return_value;
	}
	if (execute_modifier==COMMAND_EXECUTE_MODIFIER_OR){
		return !!last_command_return_value;
	}
	return 0;
}



void command_execute(const char* command){
	u32 state=COMMAND_PARSER_STATE_ARGUMENTS;
	u32 execute_modifier=COMMAND_EXECUTE_MODIFIER_ALWAYS;
	s64 last_command_return_value=0;
	command_context_t* ctx=_create_command_context();
	while (*command){
		if (COMMAND_PARSER_IS_WHITESPACE(*command)){
			for (;COMMAND_PARSER_IS_WHITESPACE(*command);command++);
			continue;
		}
		if (*command=='>'&&*(command+1)=='>'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command+=2;
			state=COMMAND_PARSER_STATE_APPEND;
			continue;
		}
		else if (*command=='>'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command++;
			state=COMMAND_PARSER_STATE_WRITE;
			continue;
		}
		else if (*command=='<'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command++;
			state=COMMAND_PARSER_STATE_READ;
			continue;
		}
		else if (*command=='|'&&*(command+1)=='|'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command+=2;
			if (_check_execute(execute_modifier,last_command_return_value)){
				_dispatch_command_context(ctx,1);
				last_command_return_value=ctx->return_value;
			}
			_delete_command_context(ctx);
			ctx=_create_command_context();
			state=COMMAND_PARSER_STATE_ARGUMENTS;
			execute_modifier=COMMAND_EXECUTE_MODIFIER_OR;
			continue;
		}
		else if (*command=='|'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command++;
			sys_io_print("error: pipe\n");goto _cleanup;
			continue;
		}
		else if (*command=='&'&&*(command+1)=='&'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command+=2;
			if (_check_execute(execute_modifier,last_command_return_value)){
				_dispatch_command_context(ctx,1);
				last_command_return_value=ctx->return_value;
			}
			_delete_command_context(ctx);
			ctx=_create_command_context();
			state=COMMAND_PARSER_STATE_ARGUMENTS;
			execute_modifier=COMMAND_EXECUTE_MODIFIER_AND;
			continue;
		}
		else if (*command=='&'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command++;
			if (_check_execute(execute_modifier,last_command_return_value)){
				_dispatch_command_context(ctx,0);
			}
			_delete_command_context(ctx);
			ctx=_create_command_context();
			state=COMMAND_PARSER_STATE_ARGUMENTS;
			execute_modifier=COMMAND_EXECUTE_MODIFIER_ALWAYS;
			last_command_return_value=0;
			continue;
		}
		else if (*command==';'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command++;
			if (_check_execute(execute_modifier,last_command_return_value)){
				_dispatch_command_context(ctx,1);
			}
			_delete_command_context(ctx);
			ctx=_create_command_context();
			state=COMMAND_PARSER_STATE_ARGUMENTS;
			execute_modifier=COMMAND_EXECUTE_MODIFIER_ALWAYS;
			last_command_return_value=0;
			continue;
		}
		char* str=NULL;
		u32 str_length=0;
		if (*command=='\"'){
			for (command++;*command!='\"';command++){
				if (!*command){
					sys_heap_dealloc(NULL,str);
					sys_io_print("error: unterminated string\n");
					goto _cleanup;
				}
				char c=*command;
				if (c!='\\'){
					goto _skip_string_control_sequence;
				}
				command++;
				if (*command=='\\'||*command=='\''||*command=='\"'){
					c=*command;
				}
				else if (*command=='e'){
					c='\e';
				}
				else if (*command=='n'){
					c='\n';
				}
				else if (*command=='r'){
					c='\r';
				}
				else if (*command=='t'){
					c='\t';
				}
				else{
					sys_heap_dealloc(NULL,str);
					sys_io_print("error: invalid string control sequence\n");
					goto _cleanup;
				}
_skip_string_control_sequence:
				str_length++;
				str=sys_heap_realloc(NULL,str,str_length);
				str[str_length-1]=c;
			}
			command++;
		}
		else if (COMMAND_PARSER_IS_VALID_CHARACTER(*command)){
			do{
				char c=*command;
				if (c!='\\'){
					goto _skip_control_sequence;
				}
				command++;
				if (*command=='\\'||*command=='>'||*command=='<'||*command=='|'||*command=='&'||*command==';'){
					c=*command;
				}
				else{
					sys_heap_dealloc(NULL,str);
					sys_io_print("error: invalid control sequence\n");
					goto _cleanup;
				}
_skip_control_sequence:
				str_length++;
				str=sys_heap_realloc(NULL,str,str_length);
				str[str_length-1]=c;
				command++;
			} while (COMMAND_PARSER_IS_VALID_CHARACTER(*command));
		}
		else{
			sys_io_print("error: unexpected character\n");
			goto _cleanup;
		}
		str=sys_heap_realloc(NULL,str,str_length+1);
		str[str_length]=0;
		if (state==COMMAND_PARSER_STATE_ARGUMENTS){
			ctx->argc++;
			ctx->argv=sys_heap_realloc(NULL,ctx->argv,ctx->argc*sizeof(char*));
			ctx->argv[ctx->argc-1]=str;
		}
		else if (state==COMMAND_PARSER_STATE_READ){
			sys_fd_t fd=sys_fd_open(cwd_fd,str,SYS_FD_FLAG_READ);
			if (SYS_IS_ERROR(fd)){
				sys_io_print("error: unable to open input file '%s': %ld\n",str,fd);
				sys_heap_dealloc(NULL,str);
				goto _cleanup;
			}
			if (ctx->flags&COMMAND_CONTEXT_FLAG_CLOSE_STDIN){
				sys_fd_close(ctx->stdin);
			}
			ctx->flags|=COMMAND_CONTEXT_FLAG_CLOSE_STDIN;
			ctx->stdin=fd;
			sys_heap_dealloc(NULL,str);
			state=COMMAND_PARSER_STATE_OPERATOR;
		}
		else if (state==COMMAND_PARSER_STATE_WRITE){
			sys_fd_t fd=sys_fd_open(cwd_fd,str,SYS_FD_FLAG_WRITE|SYS_FD_FLAG_CREATE);
			if (SYS_IS_ERROR(fd)){
				sys_io_print("error: unable to open output file '%s': %ld\n",str,fd);
				sys_heap_dealloc(NULL,str);
				goto _cleanup;
			}
			if (SYS_IS_ERROR(sys_fd_resize(fd,0,0))){
				sys_io_print("error: unable clear output file '%s'\n",str);
				sys_heap_dealloc(NULL,str);
				goto _cleanup;
			}
			if (ctx->flags&COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
				sys_fd_close(ctx->stdout);
			}
			ctx->flags|=COMMAND_CONTEXT_FLAG_CLOSE_STDOUT;
			ctx->stdout=fd;
			sys_heap_dealloc(NULL,str);
			state=COMMAND_PARSER_STATE_OPERATOR;
		}
		else if (state==COMMAND_PARSER_STATE_APPEND){
			sys_fd_t fd=sys_fd_open(cwd_fd,str,SYS_FD_FLAG_WRITE|SYS_FD_FLAG_APPEND);
			if (SYS_IS_ERROR(fd)){
				sys_io_print("error: unable to open output file '%s': %ld\n",str,fd);
				sys_heap_dealloc(NULL,str);
				goto _cleanup;
			}
			if (ctx->flags&COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
				sys_fd_close(ctx->stdout);
			}
			ctx->flags|=COMMAND_CONTEXT_FLAG_CLOSE_STDOUT;
			ctx->stdout=fd;
			sys_heap_dealloc(NULL,str);
			state=COMMAND_PARSER_STATE_OPERATOR;
		}
		else{
			sys_heap_dealloc(NULL,str);
			sys_io_print("error: invalid parser state\n");
			goto _cleanup;
		}
	}
	if (_check_execute(execute_modifier,last_command_return_value)){
		_dispatch_command_context(ctx,1);
		last_command_return_value=ctx->return_value;
	}
	sys_io_print("Return value: %p\n",last_command_return_value);
_cleanup:
	_delete_command_context(ctx);
}
