#include <command.h>
#include <string.h>
#include <user/io.h>
#include <user/system.h>
#include <user/types.h>



void shutdown_main(int argc,const char*const* argv){
	u8 flags=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-r")){
			flags|=SHUTDOWN_FLAG_RESTART;
		}
		else if (string_equal(argv[i],"-s")){
			flags|=SHUTDOWN_FLAG_SAVE_CONTEXT;
		}
		else{
			printf("shutdown: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	shutdown(flags);
}



DECLARE_COMMAND(shutdown,"shutdown [-r] [-s]");
