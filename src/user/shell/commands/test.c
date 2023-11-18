#include <command.h>
#include <sys/syscall.h>
#include <sys/types.h>



void test_main(int _,const char*const* __){
	const char* argv[]={
		"/bin/test",
		"arg0",
		"arg1"
	};
	const char* environ[]={
		"key1=value1",
		"key2=value2",
		NULL
	};
	_syscall_process_start("/bin/test",3,argv,environ,0);
}



DECLARE_COMMAND(test,"test");
