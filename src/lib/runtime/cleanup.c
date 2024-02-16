#include <sys/io/io.h>
#include <sys/types.h>



extern void __sys_fini(void) __attribute__((weak));
extern void __sys_linker_set_root_object_gcov_info(u64,u64) __attribute__((weak));
extern u64 __gcov_info_start[1];
extern u64 __gcov_info_end[1];



void _execute_fini(void){
#if KERNEL_COVERAGE_ENABLED
	__sys_linker_set_root_object_gcov_info((u64)__gcov_info_start,((u64)__gcov_info_end)-((u64)__gcov_info_start));
#endif
	__sys_fini();
}
