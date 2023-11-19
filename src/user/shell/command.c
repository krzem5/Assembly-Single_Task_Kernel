#include <command.h>
#include <cwd.h>
#include <input.h>
#include <string.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <sys/types.h>



extern const command_t* __start_commands;
extern const command_t* __stop_commands;



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
		_syscall_process_start(path,argc,argv,NULL,0);
		return;
	}
	for (const command_t*const* ptr=&__start_commands;ptr<&__stop_commands;ptr++){
		if (*ptr&&string_equal(buffer,(*ptr)->name)){
			(*ptr)->func(argc,argv);
			return;
		}
	}
	printf("%s: command not found\n",buffer);
}
