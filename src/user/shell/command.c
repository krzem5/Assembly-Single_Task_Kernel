#include <command.h>
#include <cwd.h>
#include <input.h>
#include <string.h>
#include <sys2/fd/fd.h>
#include <sys2/io/io.h>
#include <sys2/mp/event.h>
#include <sys2/mp/process.h>
#include <sys2/mp/thread.h>
#include <sys2/types.h>



static const char* search_path[]={
	".",
	"/bin",
	NULL
};



void command_execute(const char* command){
	char buffer[INPUT_BUFFER_SIZE+1];
	char* buffer_ptr=buffer;
	const char* argv[MAX_ARG_COUNT]={buffer_ptr};
	u32 argc=1;
	while (*command){
		if (*command==' '){
			argv[argc]=buffer_ptr+1;
			if (argc<MAX_ARG_COUNT){
				argc++;
			}
			*buffer_ptr=0;
		}
		else{
			*buffer_ptr=*command;
		}
		command++;
		buffer_ptr++;
	}
	*buffer_ptr=0;
	if (string_equal(buffer,"cd")){
		if (argc<2){
			sys2_io_print("cd: no input file supplied\n");
			return;
		}
		if (argc>2){
			sys2_io_print("cd: unrecognized option '%s'\n",argv[2]);
			return;
		}
		if (!cwd_change(argv[1])){
			sys2_io_print("cd: unable to change current working directory to '%s'\n",argv[1]);
		}
		return;
	}
	for (u32 i=0;search_path[i];i++){
		s64 parent_fd=sys2_fd_open(cwd_fd,search_path[i],0);
		if (parent_fd<=0){
			continue;
		}
		s64 fd=sys2_fd_open(parent_fd,argv[0],0);
		sys2_fd_close(parent_fd);
		if (fd<=0){
			continue;
		}
		char path[4096];
		sys2_fd_path(fd,path,4096);
		sys2_fd_close(fd);
		sys2_process_t process=sys2_process_start(path,argc,argv,NULL,0);
		if (!process){
			return;
		}
		sys2_event_t event=sys2_process_get_termination_event(process);
		sys2_thread_await_events(&event,1);
		return;
	}
	sys2_io_print("%s: command not found\n",buffer);
}
