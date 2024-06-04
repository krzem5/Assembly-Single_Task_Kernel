#include <shell/command.h>
#include <shell/environment.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/signal/signal.h>
#include <sys/io/io.h>
#include <sys/pipe/pipe.h>
#include <sys/types.h>



#define COMMAND_PARSER_IS_WHITESPACE(c) ((c)==' '||(c)=='\t'||(c)=='\r')
#define COMMAND_PARSER_IS_VALID_CHARACTER(c) ((c)&&!COMMAND_PARSER_IS_WHITESPACE(c)&&(c)!='>'&&(c)!='<'&&(c)!='|'&&(c)!='&'&&(c)!=';')

#define COMMAND_PARSER_STATE_ARGUMENTS 0
#define COMMAND_PARSER_STATE_OPERATOR 1
#define COMMAND_PARSER_STATE_READ 2
#define COMMAND_PARSER_STATE_WRITE 3
#define COMMAND_PARSER_STATE_APPEND 4

#define COMMAND_EXECUTE_MODIFIER_ALWAYS 0
#define COMMAND_EXECUTE_MODIFIER_AND 1
#define COMMAND_EXECUTE_MODIFIER_OR 2



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



SYS_PUBLIC void shell_interpreter_execute(shell_environment_t* env,const char* command){
	u32 state=COMMAND_PARSER_STATE_ARGUMENTS;
	u32 execute_modifier=COMMAND_EXECUTE_MODIFIER_ALWAYS;
	s64 last_command_return_value=0;
	shell_command_context_t* ctx=shell_command_context_create();
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
				shell_command_context_dispatch(ctx,env,1);
				last_command_return_value=ctx->return_value;
				if (env->close_current_session){
					goto _cleanup;
				}
			}
			shell_command_context_delete(ctx);
			ctx=shell_command_context_create();
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
			sys_fd_t pipe_fd=sys_pipe_create(NULL);
			if (SYS_IS_ERROR(pipe_fd)){
				sys_io_print("error: unable to create pipe\n");
				goto _cleanup;
			}
			if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
				sys_fd_close(ctx->stdout);
			}
			ctx->flags|=SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT;
			ctx->stdout=sys_fd_dup(pipe_fd,SYS_FD_FLAG_WRITE|SYS_FD_FLAG_CLOSE_PIPE);
			if (_check_execute(execute_modifier,last_command_return_value)){
				shell_command_context_dispatch(ctx,env,0);
			}
			shell_command_context_delete(ctx);
			ctx=shell_command_context_create();
			ctx->flags|=SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDIN;
			ctx->stdin=sys_fd_dup(pipe_fd,SYS_FD_FLAG_READ);
			sys_fd_close(pipe_fd);
			state=COMMAND_PARSER_STATE_ARGUMENTS;
			execute_modifier=COMMAND_EXECUTE_MODIFIER_ALWAYS;
			continue;
		}
		else if (*command=='&'&&*(command+1)=='&'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command+=2;
			if (_check_execute(execute_modifier,last_command_return_value)){
				shell_command_context_dispatch(ctx,env,1);
				last_command_return_value=ctx->return_value;
				if (env->close_current_session){
					goto _cleanup;
				}
			}
			shell_command_context_delete(ctx);
			ctx=shell_command_context_create();
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
				shell_command_context_dispatch(ctx,env,0);
				// sys_signal_dispatch(ctx->process,SYS_SIGNAL_INTERRUPT);
			}
			shell_command_context_delete(ctx);
			ctx=shell_command_context_create();
			state=COMMAND_PARSER_STATE_ARGUMENTS;
			execute_modifier=COMMAND_EXECUTE_MODIFIER_ALWAYS;
			last_command_return_value=0;
			continue;
		}
		else if (*command==';'||*command=='\n'){
			if (state!=COMMAND_PARSER_STATE_ARGUMENTS&&state!=COMMAND_PARSER_STATE_OPERATOR){
				sys_io_print("error: invalid parser state\n");
				goto _cleanup;
			}
			command++;
			if (_check_execute(execute_modifier,last_command_return_value)){
				shell_command_context_dispatch(ctx,env,1);
				last_command_return_value=ctx->return_value;
				if (env->close_current_session){
					goto _cleanup;
				}
			}
			shell_command_context_delete(ctx);
			ctx=shell_command_context_create();
			env->last_return_value=last_command_return_value;
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
				if (c=='$'){
					sys_heap_dealloc(NULL,str);
					sys_io_print("error: string variable expansion (unimplemented)\n");
					goto _cleanup;
				}
				else if (c!='\\'){
					goto _skip_string_control_sequence;
				}
				command++;
				if (*command=='\\'||*command=='\''||*command=='\"'||*command=='$'){
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
			str=sys_heap_realloc(NULL,str,str_length+1);
			str[str_length]=0;
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
			if (str[0]=='$'){
				char* value=shell_environment_get_variable(env,str+1,str_length-1);
				sys_heap_dealloc(NULL,str);
				if (value){
					str=value;
				}
				else{
					str=sys_heap_alloc(NULL,1);
					str[0]=0;
				}
			}
			else{
				str=sys_heap_realloc(NULL,str,str_length+1);
				str[str_length]=0;
			}
		}
		else{
			sys_io_print("error: unexpected character\n");
			goto _cleanup;
		}
		if (state==COMMAND_PARSER_STATE_ARGUMENTS){
			ctx->argc++;
			ctx->argv=sys_heap_realloc(NULL,ctx->argv,ctx->argc*sizeof(char*));
			ctx->argv[ctx->argc-1]=str;
		}
		else if (state==COMMAND_PARSER_STATE_READ){
			sys_fd_t fd=sys_fd_open(env->cwd_fd,str,SYS_FD_FLAG_READ);
			if (SYS_IS_ERROR(fd)){
				sys_io_print("error: unable to open input file '%s': %ld\n",str,fd);
				sys_heap_dealloc(NULL,str);
				goto _cleanup;
			}
			if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDIN){
				sys_fd_close(ctx->stdin);
			}
			ctx->flags|=SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDIN;
			ctx->stdin=fd;
			sys_heap_dealloc(NULL,str);
			state=COMMAND_PARSER_STATE_OPERATOR;
		}
		else if (state==COMMAND_PARSER_STATE_WRITE){
			sys_fd_t fd=sys_fd_open(env->cwd_fd,str,SYS_FD_FLAG_WRITE|SYS_FD_FLAG_CREATE);
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
			if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
				sys_fd_close(ctx->stdout);
			}
			ctx->flags|=SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT;
			ctx->stdout=fd;
			sys_heap_dealloc(NULL,str);
			state=COMMAND_PARSER_STATE_OPERATOR;
		}
		else if (state==COMMAND_PARSER_STATE_APPEND){
			sys_fd_t fd=sys_fd_open(env->cwd_fd,str,SYS_FD_FLAG_WRITE|SYS_FD_FLAG_APPEND);
			if (SYS_IS_ERROR(fd)){
				sys_io_print("error: unable to open output file '%s': %ld\n",str,fd);
				sys_heap_dealloc(NULL,str);
				goto _cleanup;
			}
			if (ctx->flags&SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT){
				sys_fd_close(ctx->stdout);
			}
			ctx->flags|=SHELL_COMMAND_CONTEXT_FLAG_CLOSE_STDOUT;
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
		shell_command_context_dispatch(ctx,env,1);
		last_command_return_value=ctx->return_value;
	}
_cleanup:
	shell_command_context_delete(ctx);
	env->last_return_value=last_command_return_value;
}
