#ifndef _KERNEL_LOCK_PROFILING_H_
#define _KERNEL_LOCK_PROFILING_H_ 1
#include <kernel/types.h>



#define LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT 12 // >=3
#define LOCK_PROFILING_MAX_LOCK_TYPES (1<<LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT)
#define LOCK_PROFILING_EARLY_LOCK_TYPES 5

#define LOCK_PROFILING_MAX_NESTED_LOCKS 16



#ifdef KERNEL_RELEASE
#define LOCK_PROFILING_LOCK_STACK
#define LOCK_PROFILING_DATA
#define lock_profiling_init(lock)
#define lock_profiling_init_lock_stack(thread)
#define lock_profiling_acquire_start(lock)
#define lock_profiling_acquire_end(lock)
#define lock_profiling_release(lock)
#else
#define LOCK_PROFILING_LOCK_STACK __lock_profiling_lock_stack_t __lock_profiling_lock_stack;
#define LOCK_PROFILING_DATA __lock_profiling_data_t __lock_profiling_data;
#define lock_profiling_init(lock) __lock_profiling_init(&((lock)->__lock_profiling_data));
#define lock_profiling_init_lock_stack(thread) __lock_profiling_init_lock_stack(&((thread)->__lock_profiling_lock_stack));
#define lock_profiling_acquire_start(lock) {__lock_profiling_acquisition_context_t __lock_profiling_acquisition_context;__lock_profiling_acquire_start(&((lock)->__lock_profiling_data),&__lock_profiling_acquisition_context);
#define lock_profiling_acquire_end(lock) __lock_profiling_acquire_end(&((lock)->__lock_profiling_data),&__lock_profiling_acquisition_context);}
#define lock_profiling_release(lock) __lock_profiling_release(&((lock)->__lock_profiling_data));
#endif



typedef struct ___LOCK_PROFILING_DATA{
	u64 alloc_address;
} __lock_profiling_data_t;



typedef struct ___LOCK_PROFILING_ACQUISITION_CONTEXT{
	u64 address;
	u64 start_time;
} __lock_profiling_acquisition_context_t;



typedef struct ___LOCK_PROFILING_LOCK_STACK{
	__lock_profiling_data_t* data[LOCK_PROFILING_MAX_NESTED_LOCKS];
	u16 size;
	u16 start;
	u16 end;
} __lock_profiling_lock_stack_t;



void __lock_profiling_init(__lock_profiling_data_t* out);



void __lock_profiling_init_lock_stack(__lock_profiling_lock_stack_t* out);



void __lock_profiling_acquire_start(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx);



void __lock_profiling_acquire_end(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx);



void __lock_profiling_release(__lock_profiling_data_t* lock);



void lock_profiling_enable_dependency_graph(void);



#endif
