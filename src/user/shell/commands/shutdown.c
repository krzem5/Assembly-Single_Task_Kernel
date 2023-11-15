#include <command.h>
#include <string.h>
#include <sys/io.h>
#include <sys/system.h>
#include <sys/types.h>



void shutdown_main(int argc,const char*const* argv){
	u8 flags=0;
	for (u32 i=1;i<argc;i++){
		if (string_equal(argv[i],"-r")){
			flags|=SHUTDOWN_FLAG_RESTART;
		}
		else{
			printf("shutdown: unrecognized option '%s'\n",argv[i]);
			return;
		}
	}
	system_shutdown(flags);
}



DECLARE_COMMAND(shutdown,"shutdown [-r]");
