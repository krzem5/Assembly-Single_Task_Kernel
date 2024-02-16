#include <sys/types.h>



extern void __sys_fini(void) __attribute__((weak));



void _execute_fini(void){
	__sys_fini();
}
