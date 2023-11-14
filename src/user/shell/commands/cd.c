#include <command.h>
#include <cwd.h>
#include <core/io.h>



void cd_main(int argc,const char*const* argv){
	if (argc<2){
		printf("cd: no input file supplied\n");
		return;
	}
	if (argc>2){
		printf("cd: unrecognized option '%s'\n",argv[2]);
		return;
	}
	if (!cwd_change(argv[1])){
		printf("cd: unable to change current working directory to '%s'\n",argv[1]);
	}
}



DECLARE_COMMAND(cd,"cd <path>");
