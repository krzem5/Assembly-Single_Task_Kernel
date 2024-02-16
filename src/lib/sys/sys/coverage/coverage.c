#if KERNEL_COVERAGE_ENABLED
#include <sys/types.h>



SYS_PUBLIC void __gcov_merge_add(void){
	return;
}
#endif
