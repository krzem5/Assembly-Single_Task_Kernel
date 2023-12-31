#include <command.h>
#include <cwd.h>
#include <input.h>
#include <string.h>
#include <sys/_kernel_syscall.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



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
			printf("cd: no input file supplied\n");
			return;
		}
		if (argc>2){
			printf("cd: unrecognized option '%s'\n",argv[2]);
			return;
		}
		if (!cwd_change(argv[1])){
			printf("cd: unable to change current working directory to '%s'\n",argv[1]);
		}
		return;
	}
	for (u32 i=0;search_path[i];i++){
		s64 parent_fd=sys_fd_open(cwd_fd,search_path[i],0);
		if (parent_fd<=0){
			continue;
		}
		s64 fd=sys_fd_open(parent_fd,argv[0],0);
		sys_fd_close(parent_fd);
		if (fd<=0){
			continue;
		}
		char path[4096];
		sys_fd_path(fd,path,4096);
		sys_fd_close(fd);
		u64 process=_syscall_process_start(path,argc,argv,NULL,0);
		if (!process){
			return;
		}
		u64 events[1]={
			_syscall_process_get_event(process)
		};
		_syscall_thread_await_events(events,1);
		return;
	}
	printf("%s: command not found\n",buffer);
}
