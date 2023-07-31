#include <command.h>
#include <cwd.h>
#include <user/fs.h>
#include <user/io.h>



void md_main(int argc,const char*const* argv){
	if (argc<2){
		printf("md: no input directory supplied\n");
		return;
	}
	if (argc>2){
		printf("md: unrecognized option '%s'\n",argv[2]);
		return;
	}
	int fd=fs_open(cwd_fd,argv[1],FS_FLAG_CREATE|FS_FLAG_DIRECTORY);
	if (fd<0){
		printf("md: unable to open directory '%s': error %d\n",argv[1],fd);
		return;
	}
	fs_close(fd);
}



DECLARE_COMMAND(md,"md <directory>");
