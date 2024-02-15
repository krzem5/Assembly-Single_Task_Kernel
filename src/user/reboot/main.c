#include <sys/system/system.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	sys_system_shutdown(SYS_SYSTEM_SHUTDOWN_FLAG_RESTART);
	return 0;
}
