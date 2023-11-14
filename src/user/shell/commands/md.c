#include <command.h>
#include <cwd.h>
#include <core/fd.h>
#include <core/io.h>



void md_main(int argc,const char*const* argv){
	if (argc<2){
		printf("md: no input directory supplied\n");
		return;
	}
	if (argc>2){
		printf("md: unrecognized option '%s'\n",argv[2]);
		return;
	}
	s64 fd=fd_open(cwd_fd,argv[1],FD_FLAG_CREATE|FD_FLAG_DIRECTORY);
	if (fd<0){
		printf("md: unable to open directory '%s': error %d\n",argv[1],fd);
		return;
	}
	fd_close(fd);
}



DECLARE_COMMAND(md,"md <directory>");
