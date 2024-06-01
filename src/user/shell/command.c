#include <command.h>
#include <cwd.h>
#include <input.h>
#include <string.h>
#include <sys/acl/acl.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/mp/event.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/string/string.h>
#include <sys/system/system.h>
#include <sys/types.h>
#include <sys/util/options.h>



#define COMMAND_PARSER_IS_WHITESPACE(c) ((c)==' '||(c)=='\t'||(c)=='\n'||(c)=='\r')
#define COMMAND_PARSER_IS_VALID_CHARACTER(c) ((c)&&!COMMAND_PARSER_IS_WHITESPACE(c)&&(c)!='>'&&(c)!='<'&&(c)!='|'&&(c)!='&'&&(c)!=';')

#define COMMAND_PARSER_STATE_ARGUMENTS 0



typedef struct _COMMAND_CONTEXT{
	u32 argc;
	char** argv;
	sys_fd_t stdin;
	sys_fd_t stdout;
	sys_fd_t stderr;
	sys_process_t process;
} command_context_t;



static const char* _search_path[]={
	".",
	"/bin",
	NULL
};



static void _handle_cd(command_context_t* ctx){
	sys_io_print("_handle_cd\n");
}



static void _handle_chroot(command_context_t* ctx){
	sys_io_print("_handle_chroot\n");
}



static void _handle_exit(command_context_t* ctx){
	if (sys_process_get_parent(0)>>16){
		sys_thread_stop(0);
	}
	sys_system_shutdown(0);
}



static const void* _internal_commands[]={
	"cd",_handle_cd,
	"chroot",_handle_chroot,
	"exit",_handle_exit,
	NULL,
};



static command_context_t* _create_command_context(void){
	command_context_t* out=sys_heap_alloc(NULL,sizeof(command_context_t));
	out->argc=0;
	out->argv=NULL;
	out->stdin=sys_io_input_fd;
	out->stdout=sys_io_output_fd;
	out->stderr=sys_io_error_fd;
	out->process=0;
	return out;
}



static void _dispatch_command_context(command_context_t* ctx){
	if (!ctx->argc){
		return;
	}
	for (u32 i=0;_internal_commands[i];i+=2){
		if (!sys_string_compare(_internal_commands[i],ctx->argv[0])){
			((void (*)(command_context_t*))(_internal_commands[i+1]))(ctx);
			goto _delete_arguments;
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
			goto _delete_arguments;
		}
		sys_acl_set_permissions(sys_process_get_handle(),ctx->process,0,SYS_PROCESS_ACL_FLAG_SWITCH_USER);
		sys_thread_start(sys_process_get_main_thread(ctx->process));
		goto _delete_arguments;
	}
	sys_io_print("error: unable to find command '%s'\n",ctx->argv[0]);
_delete_arguments:
	for (u32 i=0;i<ctx->argc;i++){
		sys_heap_dealloc(NULL,ctx->argv[i]);
	}
	sys_heap_dealloc(NULL,ctx->argv);
	ctx->argc=0;
	ctx->argv=NULL;
}



static void _await_command_context(command_context_t* ctx){
	if (ctx->process){
		sys_thread_await_event(sys_process_get_termination_event(ctx->process));
		ctx->process=0;
	}
}



static void _delete_command_context(command_context_t* ctx){
	for (u32 i=0;i<ctx->argc;i++){
		sys_heap_dealloc(NULL,ctx->argv[i]);
	}
	sys_heap_dealloc(NULL,ctx->argv);
	sys_heap_dealloc(NULL,ctx);
}



void command_execute(const char* command){
	u32 state=COMMAND_PARSER_STATE_ARGUMENTS;
	command_context_t* ctx=_create_command_context();
	while (*command){
		if (COMMAND_PARSER_IS_WHITESPACE(*command)){
			for (;COMMAND_PARSER_IS_WHITESPACE(*command);command++);
			continue;
		}
		if (*command=='>'&&*(command+1)=='>'){
			sys_io_print("error: '>>'\n");goto _cleanup;
			command+=2;
			continue;
		}
		else if (*command=='>'){
			sys_io_print("error: '>'\n");goto _cleanup;
			command++;
			continue;
		}
		else if (*command=='<'){
			sys_io_print("error: '<'\n");goto _cleanup;
			command++;
			continue;
		}
		else if (*command=='|'&&*(command+1)=='|'){
			sys_io_print("error: '||'\n");goto _cleanup;
			command+=2;
			continue;
		}
		else if (*command=='|'){
			sys_io_print("error: '|'\n");goto _cleanup;
			command++;
			continue;
		}
		else if (*command=='&'&&*(command+1)=='&'){
			sys_io_print("error: '&&'\n");goto _cleanup;
			command+=2;
			continue;
		}
		else if (*command=='&'){
			command++;
			_dispatch_command_context(ctx);
			_delete_command_context(ctx);
			ctx=_create_command_context();
			continue;
		}
		else if (*command==';'){
			command++;
			_dispatch_command_context(ctx);
			_await_command_context(ctx);
			_delete_command_context(ctx);
			ctx=_create_command_context();
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
		else{
			sys_io_print("error: unexpected character\n");
		}
	}
	_dispatch_command_context(ctx);
	_await_command_context(ctx);
_cleanup:
	_delete_command_context(ctx);
	// /***********************************/
	// char buffer[INPUT_BUFFER_SIZE+1];
	// char* buffer_ptr=buffer;
	// const char* argv[MAX_ARG_COUNT]={buffer_ptr};
	// u32 argc=1;
	// while (*command){
	// 	if (*command==' '){
	// 		argv[argc]=buffer_ptr+1;
	// 		if (argc<MAX_ARG_COUNT){
	// 			argc++;
	// 		}
	// 		*buffer_ptr=0;
	// 	}
	// 	else{
	// 		*buffer_ptr=*command;
	// 	}
	// 	command++;
	// 	buffer_ptr++;
	// }
	// *buffer_ptr=0;
	// if (!sys_string_compare(buffer,"cd")){
	// 	if (argc<2){
	// 		sys_io_print("cd: no input file supplied\n");
	// 		return;
	// 	}
	// 	if (argc>2){
	// 		sys_io_print("cd: unrecognized option '%s'\n",argv[2]);
	// 		return;
	// 	}
	// 	if (!cwd_change(argv[1])){
	// 		sys_io_print("cd: unable to change current working directory to '%s'\n",argv[1]);
	// 	}
	// 	return;
	// }
	// if (!sys_string_compare(buffer,"exit")){
	// 	if (!sys_options_parse(argc,argv,NULL)){
	// 		return;
	// 	}
	// 	if (sys_process_get_parent(0)>>16){
	// 		sys_thread_stop(0);
	// 	}
	// 	sys_system_shutdown(0);
	// 	return;
	// }
	// if (!sys_string_compare(buffer,"chroot")){
	// 	if (argc<2){
	// 		sys_io_print("chroot: no input file supplied\n");
	// 		return;
	// 	}
	// 	if (argc>2){
	// 		sys_io_print("chroot: unrecognized option '%s'\n",argv[2]);
	// 		return;
	// 	}
	// 	if (!cwd_change_root(argv[1])){
	// 		sys_io_print("chroot: unable to change current root to '%s'\n",argv[1]);
	// 	}
	// 	return;
	// }
}
