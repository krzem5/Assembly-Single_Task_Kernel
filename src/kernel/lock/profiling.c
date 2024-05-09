#include <kernel/clock/clock.h>
#include <kernel/cpu/local.h>
#include <kernel/lock/bitlock.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lock_profiling"



static u64* KERNEL_INIT_WRITE _lock_profiling_dependency_matrix=NULL;
static __lock_profiling_lock_stack_t KERNEL_INIT_WRITE _lock_profiling_global_lock_stack={
	.size=0
};
static CPU_LOCAL_DATA(__lock_profiling_lock_stack_t,_lock_profiling_cpu_local_data);



static KERNEL_INLINE __lock_profiling_lock_stack_t* KERNEL_NOCOVERAGE _get_lock_stack(void){
	if (!_lock_profiling_cpu_local_data){
		return &_lock_profiling_global_lock_stack;
	}
#ifndef KERNEL_RELEASE
	if (CPU_HEADER_DATA->current_thread){
		return &(CPU_HEADER_DATA->current_thread->__lock_profiling_lock_stack);
	}
#endif
	return CPU_LOCAL(_lock_profiling_cpu_local_data);
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init(__lock_profiling_data_t* out){
	out->alloc_address=(u64)__builtin_return_address(1);
}



void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init_lock_stack(__lock_profiling_lock_stack_t* out){
	out->size=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_start(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	if (!_lock_profiling_dependency_matrix){
		return;
	}
	ctx->address=(u64)__builtin_return_address(1);
	ctx->start_time=clock_get_time();
	__lock_profiling_lock_stack_t* stack=_get_lock_stack();
	if (stack->size>=LOCK_PROFILING_MAX_NESTED_LOCKS){
		log("\x1b[1m\x1b[38;2;41;137;255m: Lock stack too small\x1b[0m\n");
		panic("Lock profiling error");
	}
	stack->data[stack->size]=lock;
	stack->size++;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_end(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	return;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_release(__lock_profiling_data_t* lock){
	if (!_lock_profiling_dependency_matrix){
		return;
	}
	__lock_profiling_lock_stack_t* stack=_get_lock_stack();
	u64 i=0;
	for (;i<stack->size&&stack->data[i]!=lock;i++);
	if (i==stack->size){
		log("\x1b[1m\x1b[38;2;41;137;255mLock '%p' not acquired in this context\x1b[0m\n",lock);
		panic("Lock profiling error");
	}
	stack->size--;
	stack->data[i]=stack->data[stack->size];
}



void KERNEL_NOCOVERAGE KERNEL_EARLY_EXEC lock_profiling_enable_dependency_graph(void){
	_lock_profiling_dependency_matrix=(void*)(pmm_alloc(pmm_align_up_address(1<<(LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT<<1))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("lock_profiling"),0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}
