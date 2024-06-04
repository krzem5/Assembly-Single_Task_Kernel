#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/pipe/pipe.h>
#include <sys/string/string.h>
#include <sys/system/system.h>
#include <sys/types.h>



static sys_fd_t in_fd=0;
static sys_fd_t child_in_fd=0;
static sys_fd_t out_fd=0;
static sys_fd_t child_out_fd=0;



static void _input_thread(void* ctx){
	while (1){
		u8 buffer[4096];
		sys_error_t ret=sys_fd_read(in_fd,buffer,sizeof(buffer),0);
		if (!ret||SYS_IS_ERROR(ret)){
			sys_pipe_close(child_in_fd);
			return;
		}
		u64 offset=0;
		u64 remaining=ret;
		while (remaining){
			ret=sys_fd_write(child_in_fd,buffer+offset,remaining,0);
			if (!ret||SYS_IS_ERROR(ret)){
				sys_pipe_close(child_in_fd);
				return;
			}
			offset+=ret;
			remaining-=ret;
		}
	}
}



static void _output_thread(void* ctx){
	while (1){
		u8 buffer[4096];
		sys_error_t ret=sys_fd_read(child_out_fd,buffer,sizeof(buffer),0);
		if (!ret||SYS_IS_ERROR(ret)){
			sys_pipe_close(child_out_fd);
			return;
		}
		u64 offset=0;
		u64 remaining=ret;
		while (remaining){
			ret=sys_fd_write(out_fd,buffer+offset,remaining,0);
			if (!ret||SYS_IS_ERROR(ret)){
				sys_pipe_close(child_out_fd);
				return;
			}
			offset+=ret;
			remaining-=ret;
		}
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
	sys_fd_t pipe=sys_pipe_create(NULL);
	child_in_fd=sys_fd_dup(pipe,SYS_FD_FLAG_WRITE);
	sys_fd_t stdin=sys_fd_dup(pipe,SYS_FD_FLAG_READ);
	sys_fd_close(pipe);
	pipe=sys_pipe_create(NULL);
	child_out_fd=sys_fd_dup(pipe,SYS_FD_FLAG_READ);
	sys_fd_t stdout_stderr=sys_fd_dup(pipe,SYS_FD_FLAG_WRITE);
	sys_fd_close(pipe);
	sys_thread_create(_input_thread,NULL,NULL);
	sys_thread_create(_output_thread,NULL,NULL);
	sys_process_t process=sys_process_start(argv[first_argument_index],argc-first_argument_index,argv+first_argument_index,NULL,0,stdin,stdout_stderr,stdout_stderr);
	sys_thread_await_event(sys_process_get_termination_event(process));
	sys_fd_close(in_fd);
	sys_fd_close(child_in_fd);
	sys_fd_close(out_fd);
	sys_fd_close(child_out_fd);
	if (shutdown_after_process){
		sys_system_shutdown(0);
	}
	return 0;
_error:
	return 1;
}
