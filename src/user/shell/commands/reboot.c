#include <command.h>
#include <user/io.h>
#include <user/shutdown.h>



void reboot_main(int argc,const char*const* argv){
	if (argc>1){
		printf("reboot: unrecognized option '%s'\n",argv[1]);
		return;
	}
	shutdown(1);
}



DECLARE_COMMAND(reboot,"reboot");
