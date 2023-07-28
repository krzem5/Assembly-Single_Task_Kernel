#include <command.h>
#include <cwd.h>
#include <string.h>
#include <user/fs.h>
#include <user/io.h>



void rm_main(int argc,const char*const* argv){
	_Bool recursive=0;
	const char* file=NULL;
	for (u32 i=1;i<argc;i++){
		if (i<argc-1&&string_equal(argv[i],"-r")){
			recursive=1;
		}
		else if (argv[i][0]!='-'&&!file){
			file=argv[i];
		}
		else{
			printf("rm: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	int fd=fs_open(cwd_fd,file,0);
	if (fd<0){
		printf("rm: unable to open file '%s': error %d\n",file,fd);
		return;
	}
	if (!recursive){
		int err=fs_delete(fd);
		if (err<0){
			printf("rm: unable to delete file '%s': error %d\n",file,err);
			fs_close(fd);
			return;
		}
		return;
	}
}



DECLARE_COMMAND(rm,"rm <drive>");
