#include <command.h>
#include <cwd.h>
#include <sys/fd.h>
#include <sys/io.h>
#include <sys/types.h>



void cp_main(int argc,const char*const* argv){
	if (argc<2){
		printf("cp: no input file supplied\n");
		return;
	}
	if (argc<3){
		printf("cp: no output file supplied\n");
		return;
	}
	if (argc>3){
		printf("cp: unrecognized option '%s'\n",argv[3]);
		return;
	}
	s64 src_fd=fd_open(cwd_fd,argv[1],FD_FLAG_READ);
	if (src_fd<0){
		printf("cp: unable to open file '%s': error %d\n",argv[1],src_fd);
		return;
	}
	fd_close(src_fd);
}



DECLARE_COMMAND(cp,"cp <source> <destination>");
