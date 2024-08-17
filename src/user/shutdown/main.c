#include <sys/system/system.h>
#include <sys/util/options.h>



int main(int argc,const char** argv){
	if (!sys_options_parse_NEW(argc,argv,"")){
		return 1;
	}
	sys_system_shutdown(0);
	return 0;
}
