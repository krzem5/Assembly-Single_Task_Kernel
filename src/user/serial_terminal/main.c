#include <readline/readline.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/mp/process.h>
#include <sys/mp/process_group.h>
#include <sys/mp/thread.h>
#include <sys/pipe/pipe.h>
#include <sys/signal/signal.h>
#include <sys/socket/socket.h>
#include <sys/string/string.h>
#include <sys/syscall/syscall.h>
#include <sys/system/system.h>
#include <sys/types.h>



static sys_fd_t in_fd=0;
static sys_fd_t child_in_fd=0;
static sys_fd_t out_fd=0;
static sys_fd_t child_out_fd=0;
static sys_fd_t ctrl_fd=0;



static void _autocomplete_callback(readline_state_t* state,const char* prefix){
	char path[4096];
	s32 j=-1;
	for (u32 i=0;prefix[i]&&i<sizeof(path)-1;i++){
		path[i]=prefix[i];
		if (path[i]=='/'){
			j=i;
		}
	}
	path[j+1]=0;
	u32 length=sys_string_length(prefix)-j-1;
	sys_fd_t cwd_fd=sys_fd_dup(SYS_FD_DUP_CWD,0);
	sys_fd_t fd=sys_fd_open(cwd_fd,(path[0]?path:"."),0);
	sys_fd_close(cwd_fd);
	for (sys_fd_iterator_t iter=sys_fd_iter_start(fd);!SYS_IS_ERROR(iter);iter=sys_fd_iter_next(iter)){
		char name[256];
		if (SYS_IS_ERROR(sys_fd_iter_get(iter,name,256))||sys_string_compare_up_to(name,prefix+j+1,length)){
			continue;
		}
		sys_string_copy(name,path+j+1);
		readline_add_autocomplete(state,path);
	}
	sys_fd_close(fd);
}



static void _input_thread(void* ctx){
	readline_state_t state;
	readline_state_init(out_fd,4096,1024,_autocomplete_callback,&state);
	while (1){
		u8 buffer[4096];
		sys_error_t length=sys_fd_read(in_fd,buffer,sizeof(buffer),0);
		if (!length||SYS_IS_ERROR(length)){
			goto _error;
		}
		const char* ptr=(void*)buffer;
		while (length){
			u64 count=readline_process(&state,ptr,length);
			ptr+=count;
			length-=count;
			if (state.event==READLINE_EVENT_CANCEL){
				sys_signal_dispatch(sys_process_group_get(0),SYS_SIGNAL_INTERRUPT);
			}
			else if (state.event==READLINE_EVENT_LINE){
				sys_error_t ret=sys_fd_write(child_in_fd,state.line,state.line_length,0);
				if (!ret||SYS_IS_ERROR(ret)){
					goto _error;
				}
			}
		}
	}
_error:
	sys_pipe_close(child_in_fd);
	readline_state_deinit(&state);
}



static void _output_thread(void* ctx){
	while (1){
		if (SYS_IS_ERROR(sys_fd_stream(child_out_fd,&out_fd,1,0))){
			sys_pipe_close(child_out_fd);
			break;
		}
	}
}



static void _control_thread(void* ctx){
	while (1){
		u8 buffer[4096];
		sys_error_t length=sys_socket_recv(ctrl_fd,buffer,sizeof(buffer),0);
		if (SYS_IS_ERROR(length)){
			if (length==SYS_ERROR_NO_SPACE){
				continue;
			}
			break;
		}
		sys_io_print_to_fd(out_fd,"<control message: %u>\n",length);
	}
}



s64 main(u32 argc,const char*const* argv){
	bool shutdown_after_process=0;
	const char* in="/dev/ser/in";
	const char* out="/dev/ser/out";
	u32 first_argument_index=0;
	for (u32 i=1;i<argc;i++){
		if (!sys_string_compare(argv[i],"-s")){
			shutdown_after_process=1;
		}
		else if (!sys_string_compare(argv[i],"-i")){
			i++;
			if (i==argc){
				goto _error;
			}
			in=argv[i];
		}
		else if (!sys_string_compare(argv[i],"-o")){
			i++;
			if (i==argc){
				goto _error;
			}
			out=argv[i];
		}
		else if (!sys_string_compare(argv[i],"-")){
			first_argument_index=i+1;
			if (first_argument_index==argc){
				goto _error;
			}
			break;
		}
	}
	if (!first_argument_index){
		goto _error;
	}
	in_fd=sys_fd_open(0,in,SYS_FD_FLAG_READ);
	out_fd=sys_fd_open(0,out,SYS_FD_FLAG_WRITE|SYS_FD_FLAG_APPEND);
	if (SYS_IS_ERROR(in_fd)||SYS_IS_ERROR(out_fd)){
		goto _error;
	}
	sys_fd_lock(in_fd,sys_process_get_handle());
	sys_fd_lock(out_fd,sys_process_get_handle());
	sys_fd_t child_pipes[3];
	u64 devfs_syscall_table_offset=sys_syscall_get_table_offset("devfs");
	if (SYS_IS_ERROR(devfs_syscall_table_offset)||SYS_IS_ERROR(_sys_syscall1(devfs_syscall_table_offset|0x00000001,(u64)child_pipes))){
		child_pipes[0]=sys_pipe_create(NULL);
		child_pipes[1]=sys_pipe_create(NULL);
		child_pipes[2]=0;
	}
	child_in_fd=sys_fd_dup(child_pipes[0],SYS_FD_FLAG_WRITE);
	sys_fd_t stdin=sys_fd_dup(child_pipes[0],SYS_FD_FLAG_READ);
	child_out_fd=sys_fd_dup(child_pipes[1],SYS_FD_FLAG_READ);
	sys_fd_t stdout_stderr=sys_fd_dup(child_pipes[1],SYS_FD_FLAG_WRITE);
	sys_fd_close(child_pipes[0]);
	sys_fd_close(child_pipes[1]);
	sys_thread_t thread=sys_thread_create(_input_thread,NULL,NULL);
	sys_thread_set_priority(thread,SYS_THREAD_PRIORITY_REALTIME);
	sys_thread_set_name(thread,"serial_terminal.input");
	thread=sys_thread_create(_output_thread,NULL,NULL);
	sys_thread_set_name(thread,"serial_terminal.output");
	sys_thread_set_priority(thread,SYS_THREAD_PRIORITY_REALTIME);
	if (child_pipes[2]){
		ctrl_fd=child_pipes[2];
		thread=sys_thread_create(_control_thread,NULL,NULL);
		sys_thread_set_name(thread,"serial_terminal.control");
		sys_thread_set_priority(thread,SYS_THREAD_PRIORITY_REALTIME);
	}
	sys_signal_set_mask(1<<SYS_SIGNAL_INTERRUPT,1);
	sys_process_t process=sys_process_start(argv[first_argument_index],argc-first_argument_index,argv+first_argument_index,NULL,0,stdin,stdout_stderr,stdout_stderr);
	sys_thread_set_priority(sys_thread_get_handle(),SYS_THREAD_PRIORITY_LOW);
	sys_thread_await_event(sys_process_get_termination_event(process));
	sys_fd_close(in_fd);
	sys_fd_close(child_in_fd);
	sys_fd_close(out_fd);
	sys_fd_close(child_out_fd);
	sys_fd_close(ctrl_fd);
	if (shutdown_after_process){
		sys_system_shutdown(0);
	}
	return 0;
_error:
	return 1;
}
