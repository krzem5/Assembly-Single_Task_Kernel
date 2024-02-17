#include <sys/elf/elf.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/system/system.h>
#include <sys/types.h>



void main(void){
	const char*const argv[2]={
		"/bin/tree",
		"/share/test"
	};
	sys_thread_await_event(sys_process_get_termination_event(sys_process_start("/bin/tree",2,argv,NULL,0)));
	sys_system_shutdown(0);
}
