#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/mp/process.h>
#include <sys/mp/process_group.h>
#include <sys/mp/thread.h>
#include <sys/pipe/pipe.h>
#include <sys/signal/signal.h>
#include <sys/string/string.h>
#include <sys/syscall/syscall.h>
#include <sys/system/system.h>
#include <sys/types.h>



#define ESCAPE_SEQUENCE_STATE_NONE 0
#define ESCAPE_SEQUENCE_STATE_INIT 1
#define ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS 2
#define ESCAPE_SEQUENCE_STATE_CSI_INTERMEDIATE 3

#define MAX_ESCAPE_SEQUENCE_LENGTH 16



typedef struct _ESCAPE_SEQUENCE_STATE{
	u32 state;
	u32 length;
	char data[MAX_ESCAPE_SEQUENCE_LENGTH+1];
} escape_sequence_state_t;



typedef struct _INPUT_STATE{
	char* line;
	u32 line_length;
	u32 cursor;
} input_state_t;



static sys_fd_t in_fd=0;
static sys_fd_t child_in_fd=0;
static sys_fd_t out_fd=0;
static sys_fd_t child_out_fd=0;



static void _input_redraw_until_end_of_line(const input_state_t* input_state){
	sys_io_print_to_fd(out_fd,"\x1b[J%s\x1b[%uD\x1b[C",input_state->line+input_state->cursor-1,input_state->line_length-input_state->cursor+1);
}



static void _input_add_character(input_state_t* input_state,char c){
	input_state->line_length++;
	input_state->line=sys_heap_realloc(NULL,input_state->line,input_state->line_length+1);
	for (u32 i=input_state->line_length-1;i>input_state->cursor;i--){
		input_state->line[i]=input_state->line[i-1];
	}
	input_state->line[input_state->cursor]=c;
	input_state->line[input_state->line_length]=0;
	input_state->cursor++;
	_input_redraw_until_end_of_line(input_state);
}



static void _input_delete_character(input_state_t* input_state,bool is_backspace){
	if (is_backspace){
		input_state->cursor--;
	}
	if (input_state->cursor>=input_state->line_length){
		return;
	}
	for (u32 i=input_state->cursor;i<input_state->line_length;i++){
		input_state->line[i]=input_state->line[i+1];
	}
	input_state->line_length--;
	input_state->line=sys_heap_realloc(NULL,input_state->line,input_state->line_length+1);
	input_state->line[input_state->line_length]=0;
	sys_io_print_to_fd(out_fd,"\x1b[%uD",(is_backspace?2:1));
	_input_redraw_until_end_of_line(input_state);
}



static void _input_process_csi_sequence(input_state_t* input_state,const char* sequence,u32 length){
	if (!sys_string_compare(sequence,"C")){
		if (input_state->cursor<input_state->line_length){
			input_state->cursor++;
			sys_io_print_to_fd(out_fd,"\x1b[C");
		}
		return;
	}
	if (!sys_string_compare(sequence,"D")){
		if (input_state->cursor){
			input_state->cursor--;
			sys_io_print_to_fd(out_fd,"\x1b[D");
		}
		return;
	}
	if (!sys_string_compare(sequence,"H")||!sys_string_compare(sequence,"1~")||!sys_string_compare(sequence,"7~")){
		if (input_state->cursor){
			sys_io_print_to_fd(out_fd,"\x1b[%uD",input_state->cursor);
			input_state->cursor=0;
		}
		return;
	}
	if (!sys_string_compare(sequence,"F")||!sys_string_compare(sequence,"4~")||!sys_string_compare(sequence,"8~")){
		if (input_state->cursor<input_state->line_length){
			sys_io_print_to_fd(out_fd,"\x1b[%uC",input_state->line_length-input_state->cursor);
			input_state->cursor=input_state->line_length;
		}
		return;
	}
	if (!sys_string_compare(sequence,"3~")){
		_input_delete_character(input_state,0);
		return;
	}
	sys_io_print_to_fd(out_fd,"<sequence: '%s'>",sequence);
}



static void _input_thread(void* ctx){
	input_state_t input_state={
		NULL,
		0,
		0
	};
	escape_sequence_state_t escape_sequence_state={
		ESCAPE_SEQUENCE_STATE_NONE,
	};
	while (1){
		u8 buffer[4096];
		sys_error_t count=sys_fd_read(in_fd,buffer,sizeof(buffer),0);
		if (!count||SYS_IS_ERROR(count)){
			sys_pipe_close(child_in_fd);
			return;
		}
		for (u32 i=0;i<count;i++){
			if (escape_sequence_state.state==ESCAPE_SEQUENCE_STATE_INIT){
				if (buffer[i]=='['){
					escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS;
					escape_sequence_state.length=0;
					continue;
				}
				// non-CSI escape sequence
				escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_NONE;
			}
			else if (escape_sequence_state.state==ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS||escape_sequence_state.state==ESCAPE_SEQUENCE_STATE_CSI_INTERMEDIATE){
				if (escape_sequence_state.state==ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS&&buffer[i]>=0x30&&buffer[i]<=0x3f&&escape_sequence_state.length<MAX_ESCAPE_SEQUENCE_LENGTH){
					escape_sequence_state.data[escape_sequence_state.length]=buffer[i];
					escape_sequence_state.length++;
					continue;
				}
				if (buffer[i]>=0x20&&buffer[i]<=0x2f&&escape_sequence_state.length<MAX_ESCAPE_SEQUENCE_LENGTH){
					escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_CSI_INTERMEDIATE;
					escape_sequence_state.data[escape_sequence_state.length]=buffer[i];
					escape_sequence_state.length++;
					continue;
				}
				if (buffer[i]>=0x40&&buffer[i]<=0x7e&&escape_sequence_state.length<MAX_ESCAPE_SEQUENCE_LENGTH){
					escape_sequence_state.data[escape_sequence_state.length]=buffer[i];
					escape_sequence_state.length++;
					escape_sequence_state.data[escape_sequence_state.length]=0;
					_input_process_csi_sequence(&input_state,escape_sequence_state.data,escape_sequence_state.length);
					escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_NONE;
					continue;
				}
				escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_NONE;
			}
			if (buffer[i]==0x03){
				sys_signal_dispatch(sys_process_group_get(0),SYS_SIGNAL_INTERRUPT);
				sys_heap_dealloc(NULL,input_state.line);
				input_state.line=NULL;
				input_state.line_length=0;
				input_state.cursor=0;
				escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_NONE;
				continue;
			}
			if (buffer[i]==0x08||buffer[i]==0x7f){
				if (input_state.cursor){
					_input_delete_character(&input_state,1);
				}
				continue;
			}
			if (buffer[i]==0x09){
				sys_io_print_to_fd(out_fd,"<tab>");
				continue;
			}
			if (buffer[i]==0x0a||buffer[i]==0x0d){
				sys_io_print_to_fd(out_fd,"\x1b[E\n");
				input_state.line_length++;
				input_state.line=sys_heap_realloc(NULL,input_state.line,input_state.line_length);
				input_state.line[input_state.line_length-1]='\n';
				sys_error_t ret=sys_fd_write(child_in_fd,input_state.line,input_state.line_length,0);
				sys_heap_dealloc(NULL,input_state.line);
				input_state.line=NULL;
				input_state.line_length=0;
				input_state.cursor=0;
				escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_NONE;
				if (!ret||SYS_IS_ERROR(ret)){
					sys_pipe_close(child_in_fd);
					return;
				}
				continue;
			}
			if (buffer[i]==0x1b){
				escape_sequence_state.state=ESCAPE_SEQUENCE_STATE_INIT;
				continue;
			}
			_input_add_character(&input_state,buffer[i]);
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
	sys_fd_t child_pipes[2];
	u64 devfs_syscall_table_offset=sys_syscall_get_table_offset("devfs");
	if (SYS_IS_ERROR(devfs_syscall_table_offset)||SYS_IS_ERROR(_sys_syscall1(devfs_syscall_table_offset|0x00000001,(u64)child_pipes))){
		child_pipes[0]=sys_pipe_create(NULL);
		child_pipes[1]=sys_pipe_create(NULL);
	}
	child_in_fd=sys_fd_dup(child_pipes[0],SYS_FD_FLAG_WRITE);
	sys_fd_t stdin=sys_fd_dup(child_pipes[0],SYS_FD_FLAG_READ);
	child_out_fd=sys_fd_dup(child_pipes[1],SYS_FD_FLAG_READ);
	sys_fd_t stdout_stderr=sys_fd_dup(child_pipes[1],SYS_FD_FLAG_WRITE);
	sys_fd_close(child_pipes[0]);
	sys_fd_close(child_pipes[1]);
	sys_thread_create(_input_thread,NULL,NULL);
	sys_thread_create(_output_thread,NULL,NULL);
	sys_signal_set_mask(1<<SYS_SIGNAL_INTERRUPT,1);
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
