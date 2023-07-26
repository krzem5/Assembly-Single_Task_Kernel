#include <command.h>
#include <cwd.h>
#include <user/io.h>



void pwd_main(int argc,const char*const* argv){
	printf("%s\n",cwd);
}



DECLARE_COMMAND(pwd);
