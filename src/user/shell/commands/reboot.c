#include <command.h>
#include <sys/io.h>
#include <sys/system.h>



void reboot_main(int argc,const char*const* argv){
	if (argc>1){
		printf("reboot: unrecognized option '%s'\n",argv[1]);
		return;
	}
	sys_system_shutdown(SHUTDOWN_FLAG_RESTART);
}



DECLARE_COMMAND(reboot,"reboot");
