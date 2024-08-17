#ifndef _TERMINAL_CLIENT_H_
#define _TERMINAL_CLIENT_H_ 1
#include <terminal/session.h>



u16 terminal_client_get_version(terminal_session_t* session);



u32 terminal_client_get_flags(terminal_session_t* session);



u32 terminal_client_set_flags(terminal_session_t* session,u32 clear,u32 set);



bool terminal_client_get_size(terminal_session_t* session,u32* width,u32* height);



#endif
