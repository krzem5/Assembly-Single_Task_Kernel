#include <kernel/clock/clock.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "lock_profiling"



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init(__lock_profiling_data_t* out){
	out->alloc_address=(u64)__builtin_return_address(1);
}



void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init_lock_stack(__lock_profiling_lock_stack_t* out){
	out->stack_size=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_start(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	ctx->address=(u64)__builtin_return_address(1);
	ctx->start_time=clock_get_time();
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_end(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	return;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_release(__lock_profiling_data_t* lock){
	return;
}



void KERNEL_NOCOVERAGE KERNEL_EARLY_EXEC lock_profiling_enable_dependency_graph(void){
	return;
}
