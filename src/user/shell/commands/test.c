#include <command.h>
#include <cwd.h>
#include <user/io.h>



void test_main(int argc,const char*const* argv){
	*((char*)0)=0xa5;
}



DECLARE_COMMAND(test,"test");
