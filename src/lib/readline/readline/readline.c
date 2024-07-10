#include <readline/readline.h>
#include <sys/fd/fd.h>
#include <sys/format/format.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/types.h>



typedef struct _DYNAMIC_BUFFER{
	char* data;
	u32 length;
} dynamic_buffer_t;



static void _dynamic_buffer_append(dynamic_buffer_t* buffer,const void* data,u32 length){
	if (!length){
		length=sys_string_length(data);
	}
	buffer->data=sys_heap_realloc(NULL,buffer->data,buffer->length+length);
	sys_memory_copy(data,buffer->data+buffer->length,length);
	buffer->length+=length;
}



static void _emit_character(readline_state_t* state,char c){
	if (state->line_length>=state->_max_line_length){
		return;
	}
	for (u32 i=state->line_length;i>state->_cursor;i--){
		state->line[i]=state->line[i-1];
	}
	state->line[state->_cursor]=c;
	state->line_length++;
	state->_cursor++;
}



static void _remove_character(readline_state_t* state){
	state->line_length--;
	for (u32 i=state->_cursor;i<state->line_length;i++){
		state->line[i]=state->line[i+1];
	}
}



static void _redraw_line(readline_state_t* state,bool add_newline){
	dynamic_buffer_t buffer={
		NULL,
		0
	};
	char tmp[32];
	if (state->_last_character_count){
		_dynamic_buffer_append(&buffer,tmp,sys_format_string(tmp,sizeof(tmp),"\x1b[%uD",state->_last_character_count));
	}
	if (state->line_length){
		_dynamic_buffer_append(&buffer,state->line,state->line_length);
	}
	_dynamic_buffer_append(&buffer,"\x1b[0K",0);
	if (state->_cursor!=state->line_length){
		_dynamic_buffer_append(&buffer,tmp,sys_format_string(tmp,sizeof(tmp),"\x1b[%uD",state->line_length-state->_cursor));
	}
	if (add_newline){
		_dynamic_buffer_append(&buffer,"\n",1);
	}
	if (sys_fd_write(state->_output_fd,buffer.data,buffer.length,0)!=buffer.length);
	sys_heap_dealloc(NULL,buffer.data);
	state->_last_character_count=state->_cursor;
}



static void _process_csi_sequence(readline_state_t* state){
	if (!sys_string_compare(state->_escape_sequence.data,"A")){
		sys_io_print_to_fd(state->_output_fd,"<sequence: up>\n");
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"B")){
		sys_io_print_to_fd(state->_output_fd,"<sequence: down>\n");
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"C")){
		if (state->_cursor<state->line_length){
			state->_cursor++;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"D")){
		if (state->_cursor){
			state->_cursor--;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"H")||!sys_string_compare(state->_escape_sequence.data,"1~")||!sys_string_compare(state->_escape_sequence.data,"7~")){
		if (state->_cursor){
			state->_cursor=0;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"F")||!sys_string_compare(state->_escape_sequence.data,"4~")||!sys_string_compare(state->_escape_sequence.data,"8~")){
		if (state->_cursor<state->line_length){
			state->_cursor=state->line_length;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"3~")){
		if (state->_cursor<state->line_length){
			_remove_character(state);
			_redraw_line(state,0);
		}
		return;
	}
	sys_io_print_to_fd(state->_output_fd,"<sequence: '%s'>\n",state->_escape_sequence.data);
}



SYS_PUBLIC void readline_state_init(sys_fd_t output_fd,u32 max_line_length,readline_state_t* state){
	state->event=READLINE_EVENT_NONE;
	state->line=sys_heap_alloc(NULL,max_line_length+2);
	state->line_length=0;
	state->_output_fd=output_fd;
	state->_max_line_length=max_line_length;
	state->_cursor=0;
	state->_last_character_count=0;
	state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_NONE;
}



SYS_PUBLIC void readline_state_deinit(readline_state_t* state){
	sys_heap_dealloc(NULL,state->line);
	state->line=NULL;
}



SYS_PUBLIC u64 readline_process(readline_state_t* state,const char* buffer,u64 buffer_length){
	if (state->event==READLINE_EVENT_LINE){
		state->line_length=0;
		state->_cursor=0;
		state->_last_character_count=0;
	}
	state->event=READLINE_EVENT_NONE;
	for (u64 i=0;i<buffer_length;i++){
		if (state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_INIT){
			if (buffer[i]=='['){
				state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS;
				state->_escape_sequence.length=0;
				continue;
			}
			// non-CSI escape sequence
			state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_NONE;
		}
		else if (state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS||state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_CSI_INTERMEDIATE){
			if (state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS&&buffer[i]>=0x30&&buffer[i]<=0x3f&&state->_escape_sequence.length<READLINE_MAX_ESCAPE_SEQUENCE_LENGTH){
				state->_escape_sequence.data[state->_escape_sequence.length]=buffer[i];
				state->_escape_sequence.length++;
				continue;
			}
			if (buffer[i]>=0x20&&buffer[i]<=0x2f&&state->_escape_sequence.length<READLINE_MAX_ESCAPE_SEQUENCE_LENGTH){
				state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_CSI_INTERMEDIATE;
				state->_escape_sequence.data[state->_escape_sequence.length]=buffer[i];
				state->_escape_sequence.length++;
				continue;
			}
			if (buffer[i]>=0x40&&buffer[i]<=0x7e&&state->_escape_sequence.length<READLINE_MAX_ESCAPE_SEQUENCE_LENGTH){
				state->_escape_sequence.data[state->_escape_sequence.length]=buffer[i];
				state->_escape_sequence.length++;
				state->_escape_sequence.data[state->_escape_sequence.length]=0;
				_process_csi_sequence(state);
				state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_NONE;
				continue;
			}
			state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_NONE;
		}
		if (buffer[i]==0x03){
			state->event=READLINE_EVENT_CANCEL;
			return i+1;
		}
		if (buffer[i]==0x08||buffer[i]==0x7f){
			if (state->_cursor){
				state->_cursor--;
				_remove_character(state);
				_redraw_line(state,0);
			}
			continue;
		}
		if (buffer[i]==0x09){
			sys_io_print_to_fd(state->_output_fd,"<sequence: tab>");
			continue;
		}
		if (buffer[i]==0x0a||buffer[i]==0x0d){
			state->_cursor=state->line_length;
			_redraw_line(state,1);
			state->line[state->line_length]='\n';
			state->line_length++;
			state->line[state->line_length]=0;
			state->event=READLINE_EVENT_LINE;
			return i+1;
		}
		if (buffer[i]==0x1b){
			state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_INIT;
			continue;
		}
		_emit_character(state,buffer[i]);
		_redraw_line(state,0);
	}
	return buffer_length;
}
