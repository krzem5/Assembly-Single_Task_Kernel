#include <command.h>
#include <cwd.h>
#include <core/io.h>



void pwd_main(int argc,const char*const* argv){
	if (argc>1){
		printf("pwd: unrecognized option '%s'\n",argv[1]);
		return;
	}
	printf("%s\n",cwd);
}



DECLARE_COMMAND(pwd,"pwd");
