#include <command.h>
#include <cwd.h>
#include <sys/fd.h>
#include <sys/io.h>



void md_main(int argc,const char*const* argv){
	if (argc<2){
		printf("md: no input directory supplied\n");
		return;
	}
	if (argc>2){
		printf("md: unrecognized option '%s'\n",argv[2]);
		return;
	}
	s64 fd=sys_fd_open(cwd_fd,argv[1],SYS_FD_FLAG_CREATE|SYS_FD_FLAG_DIRECTORY);
	if (fd<0){
		printf("md: unable to open directory '%s': error %d\n",argv[1],fd);
		return;
	}
	sys_fd_close(fd);
}



DECLARE_COMMAND(md,"md <directory>");
