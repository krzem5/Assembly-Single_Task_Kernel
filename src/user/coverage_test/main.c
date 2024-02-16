#include <sys/elf/elf.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/system/system.h>
#include <sys/types.h>



void main(void){
	// sys_thread_await_event(sys_process_get_termination_event(sys_process_start("/bin/ls",0,NULL,NULL,0)));
	sys_system_shutdown(0);
}
