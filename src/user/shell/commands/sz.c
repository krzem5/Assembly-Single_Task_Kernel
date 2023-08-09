#include <command.h>
#include <cwd.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/types.h>



void sz_main(int argc,const char*const* argv){
	if (argc<2){
		printf("sz: no input file supplied\n");
		return;
	}
	if (argc>3){
		printf("sz: unrecognized option '%s'\n",argv[3]);
		return;
	}
	int fd=fs_open(cwd_fd,argv[1],0);
	if (fd<0){
		printf("sz: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	if (argc==2){
		printf("%lu\n",fs_seek(fd,0,FS_SEEK_END));
	}
	else{
		goto _cleanup;
	}
_cleanup:
	fs_close(fd);
}



DECLARE_COMMAND(sz,"sz <file> [<size>]");
