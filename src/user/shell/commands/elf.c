#include <command.h>
#include <cwd.h>
#include <user/elf.h>
#include <user/fs.h>
#include <user/io.h>
#include <user/thread.h>
#include <user/types.h>



void elf_main(int argc,const char*const* argv){
	if (argc<2){
		printf("elf: no input file supplied\n");
		return;
	}
	if (argc>2){
		printf("elf: unrecognized option '%s'\n",argv[2]);
		return;
	}
	int fd=fs_open(cwd_fd,argv[1],FS_FLAG_READ);
	if (fd<0){
		printf("elf: unable to open file '%s': error %d\n",argv[1],fd);
		return;
	}
	char buffer[4096];
	int error=fs_absolute_path(fd,buffer,4096);
	if (error<0){
		printf("elf: unable resolve file path: error %d\n",error);
		return;
	}
	fs_close(fd);
	if (elf_load(buffer)){
		thread_stop();
	}
	printf("elf: unable to load '%s' as an ELF executable\n",argv[1]);
}



DECLARE_COMMAND(elf,"elf <file>");
