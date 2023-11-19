#include <sys/io.h>
#include <sys/options.h>
#include <sys/system.h>



int main(int argc,const char** argv){
	u32 i=sys_options_parse(argc,argv,NULL);
	if (!i){
		return 1;
	}
	sys_system_shutdown(0);
	return 0;
}
