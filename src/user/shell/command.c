#include <command.h>
#include <cwd.h>
#include <input.h>
#include <string.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/io/io.h>
#include <sys/mp/event.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/string/string.h>
#include <sys/system/system.h>
#include <sys/types.h>
#include <sys/util/options.h>



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
	if (!sys_string_compare(buffer,"cd")){
		if (argc<2){
			sys_io_print("cd: no input file supplied\n");
			return;
		}
		if (argc>2){
			sys_io_print("cd: unrecognized option '%s'\n",argv[2]);
			return;
		}
		if (!cwd_change(argv[1])){
			sys_io_print("cd: unable to change current working directory to '%s'\n",argv[1]);
		}
		return;
	}
	if (!sys_string_compare(buffer,"exit")){
		if (!sys_options_parse(argc,argv,NULL)){
			return;
		}
		if (sys_process_get_parent(0)>>16){
			sys_thread_stop(0);
		}
		sys_system_shutdown(0);
		return;
	}
	if (!sys_string_compare(buffer,"chroot")){
		if (argc<2){
			sys_io_print("chroot: no input file supplied\n");
			return;
		}
		if (argc>2){
			sys_io_print("chroot: unrecognized option '%s'\n",argv[2]);
			return;
		}
		if (!cwd_change_root(argv[1])){
			sys_io_print("chroot: unable to change current root to '%s'\n",argv[1]);
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
		sys_fd_stat_t stat;
		if (SYS_IS_ERROR(fd)||SYS_IS_ERROR(sys_fd_stat(fd,&stat))||stat.type!=SYS_FD_STAT_TYPE_FILE){
			continue;
		}
		char path[4096];
		sys_fd_path(fd,path,4096);
		sys_fd_close(fd);
		sys_process_t process=sys_process_start(path,argc,argv,NULL,0);
		if (SYS_IS_ERROR(process)){
			break;
		}
		sys_thread_await_event(sys_process_get_termination_event(process));
		return;
	}
	sys_io_print("%s: command not found\n",buffer);
}
