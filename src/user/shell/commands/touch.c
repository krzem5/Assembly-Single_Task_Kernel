#include <command.h>
#include <cwd.h>
#include <sys/fd.h>
#include <sys/io.h>



void touch_main(int argc,const char*const* argv){
	if (argc<2){
		printf("touch: no input file supplied\n");
		return;
	}
	if (argc>2){
		printf("touch: unrecognized option '%s'\n",argv[2]);
		return;
	}
	s64 fd=sys_fd_open(cwd_fd,argv[1],FD_FLAG_CREATE);
	if (fd<0){
		printf("touch: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	sys_fd_close(fd);
}



DECLARE_COMMAND(touch,"touch <file>");
