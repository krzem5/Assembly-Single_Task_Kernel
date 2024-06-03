#include <sys/syscall/kernel_syscalls.h>
#include <sys/types.h>



extern void __sys_fini(void) __attribute__((weak));
extern void __sys_linker_set_root_object_gcov_info(u64,u64) __attribute__((weak));
extern u64 __gcov_info_start[1];
extern u64 __gcov_info_end[1];



void SYS_NORETURN SYS_NOCOVERAGE _execute_fini(void* ret){
#ifdef KERNEL_COVERAGE
	__sys_linker_set_root_object_gcov_info((u64)__gcov_info_start,((u64)__gcov_info_end)-((u64)__gcov_info_start));
#endif
	__sys_fini();
	_sys_syscall_thread_stop(0,ret);
	for (;;);
}
