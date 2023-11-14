#include <command.h>
#include <core/io.h>
#include <core/system.h>



void exit_main(int argc,const char*const* argv){
	if (argc>1){
		printf("exit: unrecognized option '%s'\n",argv[1]);
		return;
	}
	system_shutdown(0);
}



DECLARE_COMMAND(exit,"exit");
