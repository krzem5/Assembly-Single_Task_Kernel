#ifndef _TERMINAL_TERMINAL_H_
#define _TERMINAL_TERMINAL_H_ 1
#include <sys/fd/fd.h>



typedef struct _TERMINAL_SESSION{
	sys_fd_t ctrl_fd;
} terminal_session_t;



bool terminal_open_session(const char* path,terminal_session_t* out);



bool terminal_open_session_from_fd(sys_fd_t fd,terminal_session_t* out);



void terminal_close_session(terminal_session_t* session);



#endif
