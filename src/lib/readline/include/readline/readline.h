#ifndef _READLINE_READLINE_H_
#define _READLINE_READLINE_H_ 1
#include <sys/fd/fd.h>
#include <sys/types.h>



#define READLINE_EVENT_NONE 0
#define READLINE_EVENT_LINE 1
#define READLINE_EVENT_CANCEL 2

#define READLINE_ESCAPE_SEQUENCE_STATE_NONE 0
#define READLINE_ESCAPE_SEQUENCE_STATE_INIT 1
#define READLINE_ESCAPE_SEQUENCE_STATE_CSI_PARAMETERS 2
#define READLINE_ESCAPE_SEQUENCE_STATE_CSI_INTERMEDIATE 3

#define READLINE_MAX_ESCAPE_SEQUENCE_LENGTH 16



struct _READLINE_STATE;



typedef void (*readline_autocomplete_callback_t)(struct _READLINE_STATE*,const char*);



typedef struct _READLINE_AUTOCOMPLETE{
	u32 offset;
	u32 index;
	u32 length;
	char** data;
} readline_autocomplete_t;



typedef struct _READLINE_HISTORY{
	char** data;
	u32 length;
	u32 max_length;
} readline_history_t;



typedef struct _READLINE_ESCAPE_SEQUENCE_STATE{
	u32 state;
	u32 length;
	char data[READLINE_MAX_ESCAPE_SEQUENCE_LENGTH+1];
} readline_escape_sequence_state_t;



typedef struct _READLINE_STATE{
	u32 event;
	char* line;
	u32 line_length;
	readline_autocomplete_t _autocomplete;
	readline_history_t _history;
	readline_autocomplete_callback_t _autocomplete_callback;
	sys_fd_t _output_fd;
	u32 _max_line_length;
	u32 _cursor;
	u32 _history_index;
	u32 _last_character_count;
	readline_escape_sequence_state_t _escape_sequence;
} readline_state_t;



void readline_state_init(sys_fd_t output_fd,u32 max_line_length,u32 max_history_length,readline_autocomplete_callback_t autocomplete_callback,readline_state_t* state);



void readline_state_deinit(readline_state_t* state);



void readline_add_autocomplete(readline_state_t* state,const char* suffix);



u64 readline_process(readline_state_t* state,const char* buffer,u64 buffer_length);



#endif
