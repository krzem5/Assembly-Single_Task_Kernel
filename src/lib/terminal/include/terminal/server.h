#ifndef _TERMINAL_SERVER_H_
#define _TERMINAL_SERVER_H_ 1
#include <terminal/session.h>



typedef struct _TERMINAL_SERVER_STATE{
	u32 flags;
	u32 (*flag_update_callback)(u32,u32,u32);
	void (*size_inquiry_callback)(u32*);
} terminal_server_state_t;



bool terminal_server_process_packet(terminal_session_t* session,terminal_server_state_t* state);



#endif
