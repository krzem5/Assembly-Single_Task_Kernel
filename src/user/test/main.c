#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/system/system.h>
#include <sys/types.h>
#include <test/sys_acl.h>
#include <test/sys_cpu.h>
#include <test/sys_format.h>
#include <test/sys_id.h>
#include <test/sys_lib.h>
#include <test/sys_pipe.h>
#include <test/test.h>



extern void __sys_linker_dump_coverage(void) __attribute__((weak));



void main(void){
	const char*const argv[2]={
		"/bin/tree",
		"/share/test"
	};
	sys_thread_await_event(sys_process_get_termination_event(sys_process_start("/bin/tree",2,argv,NULL,0)));
	test_sys_acl();
	test_sys_cpu();
	test_sys_format();
	test_sys_id();
	test_sys_lib();
	test_sys_pipe();
	__sys_linker_dump_coverage();
	sys_system_shutdown(0);
}
