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



void reboot_main(int argc,const char*const* argv){
	if (argc>1){
		printf("reboot: unrecognized option '%s'\n",argv[1]);
		return;
	}
	shutdown(1);
}



void exit_main(int argc,const char*const* argv){
	if (argc>1){
		printf("exit: unrecognized option '%s'\n",argv[1]);
		return;
	}
	shutdown(0);
}



DECLARE_COMMAND(shutdown,"shutdown [-r]");
DECLARE_COMMAND(reboot,"reboot");
DECLARE_COMMAND(exit,"exit");
