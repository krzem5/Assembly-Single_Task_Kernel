#include <command.h>
#include <cwd.h>
#include <user/fd.h>
#include <user/io.h>
#include <user/types.h>



void cat_main(int argc,const char*const* argv){
	if (argc<2){
		printf("cat: no input file supplied\n");
		return;
	}
	if (argc>2){
		printf("cat: unrecognized option '%s'\n",argv[2]);
		return;
	}
	s64 fd=fd_open(cwd_fd,argv[1],FD_FLAG_READ);
	if (fd<0){
		printf("cat: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	char buffer[512];
	while (1){
		s64 length=fd_read(fd,buffer,512);
		if (length<0){
			printf("cat: unable to read from file '%s': error %d\n",argv[1],length);
			goto _cleanup;
		}
		if (!length){
			break;
		}
		print_buffer(buffer,length);
	}
	putchar('\n');
_cleanup:
	fd_close(fd);
}



DECLARE_COMMAND(cat,"cat <file>");
