#include <sys/io/io.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <terminal/client.h>
#include <terminal/session.h>
#include <terminal/terminal.h>



int main(int argc,const char** argv){
	terminal_session_t session;
	if (!terminal_session_open(NULL,&session)){
		sys_io_print("term: unable to open terminal session\n");
		return 1;
	}
	if (argc<2){
		u16 version=terminal_client_get_version(&session);
		u32 size[2]={0,0};
		terminal_client_get_size(&session,size,size+1);
		sys_io_print("Version: %u.%u\nFlags: %x\nSize: %ux%u\n",version>>8,version&0xff,terminal_client_get_flags(&session),size[0],size[1]);
	}
	else if (!sys_string_compare(argv[1],"autocomplete")){
		bool change=0;
		if (argc>=3&&!sys_string_compare(argv[2],"enable")){
			terminal_client_set_flags(&session,TERMINAL_FLAG_DISABLE_AUTOCOMPLETE,0);
			change=1;
		}
		else if (argc>=3&&!sys_string_compare(argv[2],"disable")){
			terminal_client_set_flags(&session,0,TERMINAL_FLAG_DISABLE_AUTOCOMPLETE);
			change=1;
		}
		sys_io_print("Autocomplete is %s%s\n",(change?"now ":""),((terminal_client_get_flags(&session)&TERMINAL_FLAG_DISABLE_AUTOCOMPLETE)?"disabled":"enabled"));
	}
	terminal_session_close(&session);
	return 0;
}
