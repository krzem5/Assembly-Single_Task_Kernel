#ifndef _KERNEL_LOCK_PROFILING_H_
#define _KERNEL_LOCK_PROFILING_H_ 1
#include <kernel/types.h>



#define LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT 12
#define LOCK_PROFILING_MAX_LOCK_TYPES (1<<LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT)

#define LOCK_PROFILING_MAX_LOCK_USES (1<<16)

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
	u32 alloc_address;
	u32 id;
} __lock_profiling_data_t;



typedef struct ___LOCK_PROFILING_ACQUISITION_CONTEXT{
	u32 stat_id;
	u64 start_ticks;
} __lock_profiling_acquisition_context_t;



typedef struct ___LOCK_PROFILING_LOCK_STACK{
	__lock_profiling_data_t* data[LOCK_PROFILING_MAX_NESTED_LOCKS];
	u64 size;
} __lock_profiling_lock_stack_t;



typedef struct _LOCK_PROFILING_DESCRIPTOR{
	u32 id;
	u32 address;
} lock_profiling_descriptor_t;



typedef struct _LOCK_PROFILING_STATS{
	u32 id;
	u32 address;
	u64 count;
	u64 ticks;
	u64 max_ticks;
} lock_profiling_stats_t;



void __lock_profiling_init(__lock_profiling_data_t* out);



void __lock_profiling_init_lock_stack(__lock_profiling_lock_stack_t* out);



void __lock_profiling_acquire_start(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx);



void __lock_profiling_acquire_end(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx);



void __lock_profiling_release(__lock_profiling_data_t* lock);



void lock_profiling_enable_dependency_graph(void);



bool lock_profiling_get_descriptor(u32 index,lock_profiling_descriptor_t* out);



bool lock_profiling_get_stats(u32 index,lock_profiling_stats_t* out);



#endif
