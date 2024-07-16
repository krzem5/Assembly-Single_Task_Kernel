#include <terminal/client.h>
#include <terminal/session.h>
#include <sys/io/io.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	terminal_session_t session;
	if (!terminal_session_open(NULL,&session)){
		sys_io_print("terminfo: unable to open terminal session\n");
		return 1;
	}
	u16 version=terminal_client_get_version(&session);
	sys_io_print("Version: %u.%u\nFlags: %x\n",version>>8,version&0xff,terminal_client_get_flags(&session));
	terminal_session_close(&session);
	return 0;
}
