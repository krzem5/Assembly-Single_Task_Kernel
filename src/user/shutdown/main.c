#include <sys2/system/system.h>
#include <sys2/util/options.h>



int main(int argc,const char** argv){
	if (!sys2_options_parse(argc,argv,NULL)){
		return 1;
	}
	sys2_system_shutdown(0);
	return 0;
}
