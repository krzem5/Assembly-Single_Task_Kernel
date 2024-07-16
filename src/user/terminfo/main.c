#include <terminal/terminal.h>
#include <sys/io/io.h>
#include <sys/types.h>



int main(int argc,const char** argv){
	terminal_session_t session;
	if (!terminal_open_session(NULL,&session)){
		sys_io_print("terminfo: unable to open terminal session\n");
		return 1;
	}
	terminal_close_session(&session);
	return 0;
}
