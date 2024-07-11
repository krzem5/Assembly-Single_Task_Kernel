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



static void _expand_history(readline_state_t* state,const char* line){
	u32 length=sys_string_length(line);
	if (!length||(state->_history.length&&!sys_memory_compare(state->_history.data[0],line,length))){
		return;
	}
	if (state->_history.length==state->_history.max_length){
		sys_heap_dealloc(NULL,state->_history.data[state->_history.length-1]);
	}
	else{
		state->_history.length++;
	}
	for (u32 i=state->_history.length-1;i;i--){
		state->_history.data[i]=state->_history.data[i-1];
	}
	state->_history.data[0]=sys_heap_alloc(NULL,length+1);
	sys_memory_copy(line,state->_history.data[0],length+1);
}



static const char* _get_autocomplete_prefix(readline_state_t* state){
	state->line[state->line_length]=0;
	const char* out=state->line;
	for (const char* ptr=state->line;*ptr;ptr++){
		if (*ptr==' '||*ptr=='\t'||*ptr==';'||*ptr=='&'||*ptr=='|'||*ptr=='<'||*ptr=='>'){
			out=ptr+1;
		}
		if (*ptr=='\"'){
			for (ptr++;*ptr!='\"';ptr++){
				if (*ptr=='\\'&&*(ptr+1)=='\"'){
					ptr++;
				}
			}
			continue;
		}
	}
	return out;
}



static void _redraw_autocomplete(readline_state_t* state){
	const char* suffix=state->_autocomplete.data[state->_autocomplete.index];
	u32 length=sys_string_length(suffix);
	if (length+state->_autocomplete.offset>state->_max_line_length){
		length=state->_max_line_length-state->_autocomplete.offset;
	}
	sys_memory_copy(suffix,state->line+state->_autocomplete.offset,length);
	state->line_length=state->_autocomplete.offset+length;
	state->_cursor=state->line_length;
	_redraw_line(state,0);
}



static void _start_autocomplete(readline_state_t* state){
	if (!state->_autocomplete_callback){
		return;
	}
	state->_autocomplete.offset=state->line_length;
	state->_autocomplete.index=0;
	state->_autocomplete.length=0;
	state->_autocomplete.data=NULL;
	state->_autocomplete_callback(state,_get_autocomplete_prefix(state));
	if (!state->_autocomplete.length){
		return;
	}
	_redraw_autocomplete(state);
}



static void _prev_autocomplete(readline_state_t* state){
	state->_autocomplete.index=(state->_autocomplete.index?state->_autocomplete.index:state->_autocomplete.length)-1;
	_redraw_autocomplete(state);
}



static void _next_autocomplete(readline_state_t* state){
	state->_autocomplete.index=(state->_autocomplete.index+1>=state->_autocomplete.length?0:state->_autocomplete.index+1);
	_redraw_autocomplete(state);
}



static void _keep_autocomplete(readline_state_t* state){
	for (u32 i=0;i<state->_autocomplete.length;i++){
		sys_heap_dealloc(NULL,state->_autocomplete.data[i]);
	}
	sys_heap_dealloc(NULL,state->_autocomplete.data);
	state->_autocomplete.length=0;
}



static void _cancel_autocomplete(readline_state_t* state){
	_keep_autocomplete(state);
	state->line_length=state->_autocomplete.offset;
	state->_cursor=state->line_length;
	_redraw_line(state,0);
}



static void _process_csi_sequence(readline_state_t* state){
	if (!sys_string_compare(state->_escape_sequence.data,"A")){
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		if (!state->_history.length){
			return;
		}
		if (state->_history_index||(state->line_length&&!sys_memory_compare(state->line,state->_history.data[state->_history_index],state->line_length))){
			state->_history_index++;
		}
		if (state->_history_index>=state->_history.length){
			state->_history_index=state->_history.length-1;
			return;
		}
		u32 length=sys_string_length(state->_history.data[state->_history_index]);
		sys_memory_copy(state->_history.data[state->_history_index],state->line,length);
		state->line_length=length;
		state->_cursor=length;
		_redraw_line(state,0);
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"B")){
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		if (!state->_history.length||!state->_history_index){
			return;
		}
		state->_history_index--;
		u32 length=sys_string_length(state->_history.data[state->_history_index]);
		sys_memory_copy(state->_history.data[state->_history_index],state->line,length);
		state->line_length=length;
		state->_cursor=length;
		_redraw_line(state,0);
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"C")){
		if (state->_cursor<state->line_length){
			if (state->_autocomplete.length){
				_keep_autocomplete(state);
			}
			state->_cursor++;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"D")){
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		if (state->_cursor){
			state->_cursor--;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"H")||!sys_string_compare(state->_escape_sequence.data,"1~")||!sys_string_compare(state->_escape_sequence.data,"7~")){
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		if (state->_cursor){
			state->_cursor=0;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"F")||!sys_string_compare(state->_escape_sequence.data,"4~")||!sys_string_compare(state->_escape_sequence.data,"8~")){
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		if (state->_cursor<state->line_length){
			state->_cursor=state->line_length;
			_redraw_line(state,0);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"Z")){
		if (state->_autocomplete.length){
			_prev_autocomplete(state);
		}
		return;
	}
	if (!sys_string_compare(state->_escape_sequence.data,"3~")){
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		if (state->_cursor<state->line_length){
			_remove_character(state);
			_redraw_line(state,0);
		}
		return;
	}
	sys_io_print_to_fd(state->_output_fd,"<sequence: '%s'>\n",state->_escape_sequence.data);
}



SYS_PUBLIC void readline_state_init(sys_fd_t output_fd,u32 max_line_length,u32 max_history_length,readline_autocomplete_callback_t autocomplete_callback,readline_state_t* state){
	state->event=READLINE_EVENT_NONE;
	state->line=sys_heap_alloc(NULL,max_line_length+2);
	state->line_length=0;
	state->_autocomplete.offset=0;
	state->_autocomplete.index=0;
	state->_autocomplete.length=0;
	state->_autocomplete.data=NULL;
	state->_history.data=sys_heap_alloc(NULL,max_history_length*sizeof(char*));
	state->_history.length=0;
	state->_history.max_length=max_history_length;
	state->_autocomplete_callback=autocomplete_callback;
	state->_output_fd=output_fd;
	state->_max_line_length=max_line_length;
	state->_cursor=0;
	state->_history_index=0;
	state->_last_character_count=0;
	state->_escape_sequence.state=READLINE_ESCAPE_SEQUENCE_STATE_NONE;
}



SYS_PUBLIC void readline_state_deinit(readline_state_t* state){
	sys_heap_dealloc(NULL,state->line);
	state->line=NULL;
	for (u32 i=0;i<state->_history.length;i++){
		sys_heap_dealloc(NULL,state->_history.data[i]);
	}
	sys_heap_dealloc(NULL,state->_history.data);
	state->_history.data=NULL;
}



SYS_PUBLIC void readline_add_autocomplete(readline_state_t* state,const char* suffix){
	state->_autocomplete.length++;
	state->_autocomplete.data=sys_heap_realloc(NULL,state->_autocomplete.data,state->_autocomplete.length*sizeof(char*));
	state->_autocomplete.data[state->_autocomplete.length-1]=sys_string_duplicate(suffix);
}



SYS_PUBLIC u64 readline_process(readline_state_t* state,const char* buffer,u64 buffer_length){
	if (state->event==READLINE_EVENT_LINE){
		state->line_length=0;
		state->_cursor=0;
		state->_history_index=0;
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
			if (state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_INIT&&state->_autocomplete.length){
				_cancel_autocomplete(state);
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
			if (state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_INIT&&state->_autocomplete.length){
				_cancel_autocomplete(state);
			}
		}
		if (buffer[i]==0x03){
			state->event=READLINE_EVENT_CANCEL;
			return i+1;
		}
		if (buffer[i]==0x08||buffer[i]==0x7f){
			if (state->_autocomplete.length){
				_keep_autocomplete(state);
			}
			if (state->_cursor){
				state->_cursor--;
				_remove_character(state);
				_redraw_line(state,0);
			}
			continue;
		}
		if (buffer[i]==0x09){
			if (!state->_autocomplete.length){
				_start_autocomplete(state);
			}
			else{
				_next_autocomplete(state);
			}
			continue;
		}
		if (buffer[i]==0x0a||buffer[i]==0x0d){
			state->_cursor=state->line_length;
			_redraw_line(state,1);
			state->line[state->line_length]=0;
			_expand_history(state,state->line);
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
		if (state->_autocomplete.length){
			_keep_autocomplete(state);
		}
		_emit_character(state,buffer[i]);
		_redraw_line(state,0);
	}
	if (state->_escape_sequence.state==READLINE_ESCAPE_SEQUENCE_STATE_INIT&&state->_autocomplete.length){
		_cancel_autocomplete(state);
	}
	return buffer_length;
}
