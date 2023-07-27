#include <command.h>
#include <string.h>
#include <user/io.h>
#include <user/shutdown.h>



void shutdown_main(int argc,const char*const* argv){
	_Bool reset=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-r")){
			reset=1;
		}
		else{
			printf("shutdown: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	shutdown(reset);
}



DECLARE_COMMAND(shutdown,"shutdown [-r]");
