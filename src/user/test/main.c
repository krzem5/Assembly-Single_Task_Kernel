#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/system/system.h>
#include <sys/types.h>
#include <test/sys_lib.h>
#include <test/test.h>



extern void __sys_linker_dump_coverage(void) __attribute__((weak));



void main(void){
	const char*const argv[2]={
		"/bin/tree",
		"/share/test"
	};
	sys_thread_await_event(sys_process_get_termination_event(sys_process_start("/bin/tree",2,argv,NULL,0)));
	test_sys_lib();
	// u64 test_pass_count=0;
	// u64 test_fail_count=0;
	__sys_linker_dump_coverage();
	sys_system_shutdown(0);
}
