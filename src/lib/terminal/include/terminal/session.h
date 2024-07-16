#ifndef _TERMINAL_SESSION_H_
#define _TERMINAL_SESSION_H_ 1
#include <sys/fd/fd.h>



typedef struct _TERMINAL_SESSION{
	sys_fd_t ctrl_fd;
} terminal_session_t;



bool terminal_session_open(const char* path,terminal_session_t* out);



bool terminal_session_open_from_fd(sys_fd_t fd,terminal_session_t* out);



void terminal_session_close(terminal_session_t* session);



u32 terminal_session_recv_packet(terminal_session_t* session,void* buffer,u32 buffer_size,u8 type);



bool terminal_session_send_packet(terminal_session_t* session,const void* buffer,u32 buffer_size);



#endif
